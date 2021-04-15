//
// Created by tymbys on 11.04.2021.
//

#include "WebRTC.h"

GMainLoop * WebRTC::loop_;
GstElement * WebRTC::pipe1_, * WebRTC::webrtc1_;
GObject * WebRTC::send_channel_, * WebRTC::receive_channel_;

SoupWebsocketConnection * WebRTC::ws_conn_ = NULL;
AppState WebRTC::app_state_ = APP_STATE_UNKNOWN;
const gchar * WebRTC::peer_id_ = "1234";//NULL;
const gchar * WebRTC::server_url_ = "wss://tymbys.com:8443";  // "wss://185.216.231.2:8443"; //"wss://webrtc.nirbheek.in:8443";
gboolean WebRTC::disable_ssl_ = FALSE;
gboolean WebRTC::remote_is_offerer_ = FALSE;


gboolean WebRTC::cleanup_and_quit_loop(const gchar *msg, enum AppState state) {
    if (msg)
        gst_printerr("%s\n", msg);
    if (state > 0)
        app_state_ = state;

    if (ws_conn_) {
        if (soup_websocket_connection_get_state(ws_conn_) ==
            SOUP_WEBSOCKET_STATE_OPEN)
            /* This will call us again */
            soup_websocket_connection_close(ws_conn_, 1000, "");
        else
            g_object_unref(ws_conn_);
    }

    if (loop_) {
        g_main_loop_quit(loop_);
        loop_ = NULL;
    }

    /* To allow usage as a GSourceFunc */
    return G_SOURCE_REMOVE;
}

gchar* WebRTC::get_string_from_json_object(JsonObject *object) {
    JsonNode *root;
    JsonGenerator *generator;
    gchar *text;

    /* Make it the root node */
    root = json_node_init_object(json_node_alloc(), object);
    generator = json_generator_new();
    json_generator_set_root(generator, root);
    text = json_generator_to_data(generator, NULL);

    /* Release everything */
    g_object_unref(generator);
    json_node_free(root);
    return text;
}


void WebRTC::send_ice_candidate_message(GstElement *webrtc G_GNUC_UNUSED, guint mlineindex,
                           gchar *candidate, gpointer user_data G_GNUC_UNUSED) {
    gchar *text;
    JsonObject *ice, *msg;

    if (app_state_ < PEER_CALL_NEGOTIATING) {
        cleanup_and_quit_loop("Can't send ICE, not in call", APP_STATE_ERROR);
        return;
    }

    ice = json_object_new();
    json_object_set_string_member(ice, "candidate", candidate);
    json_object_set_int_member(ice, "sdpMLineIndex", mlineindex);
    msg = json_object_new();
    json_object_set_object_member(msg, "ice", ice);
    text = get_string_from_json_object(msg);
    json_object_unref(msg);

    soup_websocket_connection_send_text(ws_conn_, text);
    g_free(text);
}

void WebRTC::send_sdp_to_peer(GstWebRTCSessionDescription *desc) {
    gchar *text;
    JsonObject *msg, *sdp;

    if (app_state_ < PEER_CALL_NEGOTIATING) {
        cleanup_and_quit_loop("Can't send SDP to peer, not in call",
                              APP_STATE_ERROR);
        return;
    }

    text = gst_sdp_message_as_text(desc->sdp);
    sdp = json_object_new();

    if (desc->type == GST_WEBRTC_SDP_TYPE_OFFER) {
        gst_print("Sending offer:\n%s\n", text);
        json_object_set_string_member(sdp, "type", "offer");
    } else if (desc->type == GST_WEBRTC_SDP_TYPE_ANSWER) {
        gst_print("Sending answer:\n%s\n", text);
        json_object_set_string_member(sdp, "type", "answer");
    } else {
        g_assert_not_reached ();
    }

    json_object_set_string_member(sdp, "sdp", text);
    g_free(text);

    msg = json_object_new();
    json_object_set_object_member(msg, "sdp", sdp);
    text = get_string_from_json_object(msg);
    json_object_unref(msg);

    soup_websocket_connection_send_text(ws_conn_, text);
    g_free(text);
}

/* Offer created by our pipeline, to be sent to the peer */
void WebRTC::on_offer_created(GstPromise *promise, gpointer user_data) {
    GstWebRTCSessionDescription *offer = NULL;
    const GstStructure *reply;

    g_assert_cmphex (app_state_, ==, PEER_CALL_NEGOTIATING);

    g_assert_cmphex (gst_promise_wait(promise), ==, GST_PROMISE_RESULT_REPLIED);
    reply = gst_promise_get_reply(promise);
    gst_structure_get(reply, "offer",
                      GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &offer, NULL);
    gst_promise_unref(promise);

    promise = gst_promise_new();
    g_signal_emit_by_name(webrtc1_, "set-local-description", offer, promise);
    gst_promise_interrupt(promise);
    gst_promise_unref(promise);

    /* Send offer to peer */
    send_sdp_to_peer(offer);
    gst_webrtc_session_description_free(offer);
}

void WebRTC::on_negotiation_needed(GstElement *element, gpointer user_data) {
    app_state_ = PEER_CALL_NEGOTIATING;

    if (remote_is_offerer_) {
        gchar *msg = g_strdup_printf("OFFER_REQUEST");
        soup_websocket_connection_send_text(ws_conn_, msg);
        g_free(msg);
    } else {
        GstPromise *promise;
        promise =
                gst_promise_new_with_change_func(on_offer_created, user_data, NULL);;
        g_signal_emit_by_name(webrtc1_, "create-offer", NULL, promise);
    }
}

#define STUN_SERVER " stun-server=stun://stun.l.google.com:19302 "
#define RTP_CAPS_OPUS "application/x-rtp,media=audio,encoding-name=OPUS,payload="
#define RTP_CAPS_VP8 "application/x-rtp,media=video,encoding-name=VP8,payload="

void WebRTC::data_channel_on_error(GObject *dc, gpointer user_data) {
    cleanup_and_quit_loop("Data channel error", APP_STATE_UNKNOWN);
}

void WebRTC::data_channel_on_open(GObject *dc, gpointer user_data) {
    GBytes *bytes = g_bytes_new("data", strlen("data"));
    gst_print("data channel opened\n");
    g_signal_emit_by_name(dc, "send-string", "Hi! from GStreamer");
    g_signal_emit_by_name(dc, "send-data", bytes);
    g_bytes_unref(bytes);
}

void WebRTC::data_channel_on_close(GObject *dc, gpointer user_data) {
    cleanup_and_quit_loop("Data channel closed", APP_STATE_UNKNOWN);
}

void WebRTC::data_channel_on_message_string(GObject *dc, gchar *str, gpointer user_data) {
//    gst_print("Received data channel message: %s\n", str);

    std::string json(str);// = "{\"SET\": {\"POWER\": 0.65, \"DIRECTION\": 0.8}}";
    std::map<CMD_t, DATA_t> ret_parse_data;
    if (Utils::ParseData(json, ret_parse_data) >= 0) {

        std::map<CMD_t, DATA_t> ret_processor;
        PROCESSOR->CleanConfigMotor();
        PROCESSOR->Run(ret_parse_data, ret_processor);
        PROCESSOR->RunDeferredCMD();

    }
}

void WebRTC::connect_data_channel_signals(GObject *data_channel) {
    g_signal_connect (data_channel, "on-error",
                      G_CALLBACK(data_channel_on_error), NULL);
    g_signal_connect (data_channel, "on-open", G_CALLBACK(data_channel_on_open),
                      NULL);
    g_signal_connect (data_channel, "on-close",
                      G_CALLBACK(data_channel_on_close), NULL);
    g_signal_connect (data_channel, "on-message-string",
                      G_CALLBACK(data_channel_on_message_string), NULL);
}

void WebRTC::on_data_channel(GstElement *webrtc, GObject *data_channel,
                gpointer user_data) {
    connect_data_channel_signals(data_channel);
    receive_channel_ = data_channel;
}

void WebRTC::on_ice_gathering_state_notify(GstElement *webrtcbin, GParamSpec *pspec,
                              gpointer user_data) {
    GstWebRTCICEGatheringState ice_gather_state;
    const gchar *new_state = "unknown";

    g_object_get(webrtcbin, "ice-gathering-state", &ice_gather_state, NULL);
    switch (ice_gather_state) {
        case GST_WEBRTC_ICE_GATHERING_STATE_NEW:
            new_state = "new";
            break;
        case GST_WEBRTC_ICE_GATHERING_STATE_GATHERING:
            new_state = "gathering";
            break;
        case GST_WEBRTC_ICE_GATHERING_STATE_COMPLETE:
            new_state = "complete";
            break;
    }
    gst_print("ICE gathering state changed to %s\n", new_state);
}

gboolean WebRTC::start_pipeline(void) {
    GstStateChangeReturn ret;
    GError *error = NULL;

//    pipe1 =
//            gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "
//                             STUN_SERVER
//                             "videotestsrc is-live=true pattern=ball ! videoconvert ! queue ! vp8enc deadline=1 ! rtpvp8pay ! "
//                             "queue ! " RTP_CAPS_VP8 "96 ! sendrecv. "
//                             "audiotestsrc is-live=true wave=red-noise ! audioconvert ! audioresample ! queue ! opusenc ! rtpopuspay ! "
//                             "queue ! " RTP_CAPS_OPUS "97 ! sendrecv. ", &error);


//    pipe1_ =
//            gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "
//                             STUN_SERVER
//                             "v4l2src device=/dev/video0 ! "
//                             "video/x-raw,width=640,height=480 ! "
//                             "videoconvert ! queue ! vp8enc deadline=1 ! rtpvp8pay ! "
//                             "queue ! " RTP_CAPS_VP8 "96 ! sendrecv. "
//                             "audiotestsrc is-live=true wave=red-noise ! audioconvert ! audioresample ! queue ! opusenc ! rtpopuspay ! "
//                             "queue ! " RTP_CAPS_OPUS "97 ! sendrecv. ", &error);

    pipe1_ =
            gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "
                             STUN_SERVER
                             "v4l2src device=/dev/video0 ! "
                             "video/x-raw,width=640,height=480 ! "
                             "videoconvert ! queue ! vp8enc deadline=1 ! rtpvp8pay ! "
                             "queue ! " RTP_CAPS_VP8 "96 ! sendrecv. "
//                             "audiotestsrc is-live=true wave=red-noise ! audioconvert ! audioresample ! queue ! opusenc ! rtpopuspay ! "
//                             "queue ! " RTP_CAPS_OPUS "97 ! sendrecv. "
                             , &error);


/*
  pipe1 =
      gst_parse_launch ("webrtcbin name=webrtcbin  stun-server=stun://stun.l.google.com:19302 "
                                  "v4l2src device=/dev/video0 "
                                  "! video/x-raw,width=1024,height=720,framerate=1/30 "
//                                  "! video/x-raw "
                                  "! videoconvert name=ocvvideosrc "
                                  "! video/x-raw,format=BGRA "
                                  "! videoconvert "
                                  "! queue max-size-buffers=1 "
                                  "! x264enc speed-preset=ultrafast tune=zerolatency key-int-max=15 "
                                  "! video/x-h264,profile=constrained-baseline "
                                  "! queue max-size-time=0 "
                                  "! h264parse "
                                  "! rtph264pay config-interval=-1 name=payloader pt=96 "
                                  "! application/x-rtp,media=video,encoding-name=H264,payload=96 "
//                                  "! capssetter caps=\"application/x-rtp,profile-level-id=(string)42c01f,media=(string)video,encoding-name=(string)H264,payload=(int)96\" "
                                  "! capssetter caps=\"application/x-rtp,media=(string)video,encoding-name=(string)H264,payload=(int)96\" "
                                  "! webrtcbin. "
                                  , &error);
*/

    if (error) {
        gst_printerr("Failed to parse launch: %s\n", error->message);
        g_error_free(error);
        goto err;
    }

    webrtc1_ = gst_bin_get_by_name(GST_BIN (pipe1_), "sendrecv");
    g_assert_nonnull (webrtc1_);

    /* This is the gstwebrtc entry point where we create the offer and so on. It
     * will be called when the pipeline goes to PLAYING. */
    g_signal_connect (webrtc1_, "on-negotiation-needed",
                      G_CALLBACK(on_negotiation_needed), NULL);
    /* We need to transmit this ICE candidate to the browser via the websockets
     * signalling server. Incoming ice candidates from the browser need to be
     * added by us too, see on_server_message() */
    g_signal_connect (webrtc1_, "on-ice-candidate",
                      G_CALLBACK(send_ice_candidate_message), NULL);
    g_signal_connect (webrtc1_, "notify::ice-gathering-state",
                      G_CALLBACK(on_ice_gathering_state_notify), NULL);

    gst_element_set_state(pipe1_, GST_STATE_READY);

    g_signal_emit_by_name(webrtc1_, "create-data-channel", "channel", NULL,
                          &send_channel_);
    if (send_channel_) {
        gst_print("Created data channel\n");
        connect_data_channel_signals(send_channel_);
    } else {
        gst_print("Could not create data channel, is usrsctp available?\n");
    }

    g_signal_connect (webrtc1_, "on-data-channel", G_CALLBACK(on_data_channel),
                      NULL);
    /* Incoming streams will be exposed via this signal */
//    g_signal_connect (webrtc1, "pad-added", G_CALLBACK(on_incoming_stream), pipe1);
    /* Lifetime is the same as the pipeline itself */
    gst_object_unref(webrtc1_);

    gst_print("Starting pipeline\n");
    ret = gst_element_set_state(GST_ELEMENT (pipe1_), GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
        goto err;

    return TRUE;

    err:
    if (pipe1_)
        g_clear_object (&pipe1_);
    if (webrtc1_)
        webrtc1_ = NULL;
    return FALSE;
}

gboolean WebRTC::setup_call(void) {
    gchar *msg;

    if (soup_websocket_connection_get_state(ws_conn_) !=
        SOUP_WEBSOCKET_STATE_OPEN)
        return FALSE;

    if (!peer_id_)
        return FALSE;

    gst_print("Setting up signalling server call with %s\n", peer_id_);
    app_state_ = PEER_CONNECTING;
    msg = g_strdup_printf("SESSION %s", peer_id_);
    soup_websocket_connection_send_text(ws_conn_, msg);
    g_free(msg);
    return TRUE;
}

gboolean WebRTC::register_with_server(void) {
    gchar *hello;
    gint32 our_id;

    if (soup_websocket_connection_get_state(ws_conn_) !=
        SOUP_WEBSOCKET_STATE_OPEN)
        return FALSE;

    our_id = g_random_int_range(10, 10000);
    gst_print("Registering id %i with server\n", our_id);
    app_state_ = SERVER_REGISTERING;

    /* Register with the server with a random integer id. Reply will be received
     * by on_server_message() */
    hello = g_strdup_printf("HELLO %i", our_id);
    soup_websocket_connection_send_text(ws_conn_, hello);
    g_free(hello);

    return TRUE;
}

void WebRTC::on_server_closed(SoupWebsocketConnection *conn G_GNUC_UNUSED,
                 gpointer user_data G_GNUC_UNUSED) {
    app_state_ = SERVER_CLOSED;
    cleanup_and_quit_loop("Server connection closed", APP_STATE_UNKNOWN);
}

/* Answer created by our pipeline, to be sent to the peer */
void WebRTC::on_answer_created(GstPromise *promise, gpointer user_data) {
    GstWebRTCSessionDescription *answer = NULL;
    const GstStructure *reply;

    g_assert_cmphex (app_state_, ==, PEER_CALL_NEGOTIATING);

    g_assert_cmphex (gst_promise_wait(promise), ==, GST_PROMISE_RESULT_REPLIED);
    reply = gst_promise_get_reply(promise);
    gst_structure_get(reply, "answer",
                      GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &answer, NULL);
    gst_promise_unref(promise);

    promise = gst_promise_new();
    g_signal_emit_by_name(webrtc1_, "set-local-description", answer, promise);
    gst_promise_interrupt(promise);
    gst_promise_unref(promise);

    /* Send answer to peer */
    send_sdp_to_peer(answer);
    gst_webrtc_session_description_free(answer);
}

void WebRTC::on_offer_set(GstPromise *promise, gpointer user_data) {
    gst_promise_unref(promise);
    promise = gst_promise_new_with_change_func(on_answer_created, NULL, NULL);
    g_signal_emit_by_name(webrtc1_, "create-answer", NULL, promise);
}

void WebRTC::on_offer_received(GstSDPMessage *sdp) {
    GstWebRTCSessionDescription *offer = NULL;
    GstPromise *promise;

    offer = gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_OFFER, sdp);
    g_assert_nonnull (offer);

    /* Set remote description on our pipeline */
    {
        promise = gst_promise_new_with_change_func(on_offer_set, NULL, NULL);
        g_signal_emit_by_name(webrtc1_, "set-remote-description", offer, promise);
    }
    gst_webrtc_session_description_free(offer);
}

/* One mega message handler for our asynchronous calling mechanism */
void WebRTC::on_server_message(SoupWebsocketConnection *conn, SoupWebsocketDataType type,
                  GBytes *message, gpointer user_data) {
    gchar *text;

    switch (type) {
        case SOUP_WEBSOCKET_DATA_BINARY:
            gst_printerr("Received unknown binary message, ignoring\n");
            return;
        case SOUP_WEBSOCKET_DATA_TEXT: {
            gsize size;
            const gchar *data = static_cast<const gchar *>(g_bytes_get_data(message, &size));
            /* Convert to NULL-terminated string */
            text = g_strndup(data, size);
            break;
        }
        default:
            g_assert_not_reached ();
    }

    /* Server has accepted our registration, we are ready to send commands */
    if (g_strcmp0(text, "HELLO") == 0) {
        if (app_state_ != SERVER_REGISTERING) {
            cleanup_and_quit_loop("ERROR: Received HELLO when not registering",
                                  APP_STATE_ERROR);
            goto out;
        }
        app_state_ = SERVER_REGISTERED;
        gst_print("Registered with server\n");
        /* Ask signalling server to connect us with a specific peer */
        if (!setup_call()) {
            cleanup_and_quit_loop("ERROR: Failed to setup call", PEER_CALL_ERROR);
            goto out;
        }
        /* Call has been setup by the server, now we can start negotiation */
    } else if (g_strcmp0(text, "SESSION_OK") == 0) {
        if (app_state_ != PEER_CONNECTING) {
            cleanup_and_quit_loop("ERROR: Received SESSION_OK when not calling",
                                  PEER_CONNECTION_ERROR);
            goto out;
        }

        app_state_ = PEER_CONNECTED;
        /* Start negotiation (exchange SDP and ICE candidates) */
        if (!start_pipeline())
            cleanup_and_quit_loop("ERROR: failed to start pipeline",
                                  PEER_CALL_ERROR);
        /* Handle errors */
    } else if (g_str_has_prefix(text, "ERROR")) {
        switch (app_state_) {
            case SERVER_CONNECTING:
                app_state_ = SERVER_CONNECTION_ERROR;
                break;
            case SERVER_REGISTERING:
                app_state_ = SERVER_REGISTRATION_ERROR;
                break;
            case PEER_CONNECTING:
                app_state_ = PEER_CONNECTION_ERROR;
                break;
            case PEER_CONNECTED:
            case PEER_CALL_NEGOTIATING:
                app_state_ = PEER_CALL_ERROR;
                break;
            default:
                app_state_ = APP_STATE_ERROR;
        }
        cleanup_and_quit_loop(text, APP_STATE_UNKNOWN);
        /* Look for JSON messages containing SDP and ICE candidates */
    } else {
        JsonNode *root;
        JsonObject *object, *child;
        JsonParser *parser = json_parser_new();
        if (!json_parser_load_from_data(parser, text, -1, NULL)) {
            gst_printerr("Unknown message '%s', ignoring", text);
            g_object_unref(parser);
            goto out;
        }

        root = json_parser_get_root(parser);
        if (!JSON_NODE_HOLDS_OBJECT (root)) {
            gst_printerr("Unknown json message '%s', ignoring", text);
            g_object_unref(parser);
            goto out;
        }

        object = json_node_get_object(root);
        /* Check type of JSON message */
        if (json_object_has_member(object, "sdp")) {
            int ret;
            GstSDPMessage *sdp;
            const gchar *text, *sdptype;
            GstWebRTCSessionDescription *answer;

            g_assert_cmphex (app_state_, ==, PEER_CALL_NEGOTIATING);

            child = json_object_get_object_member(object, "sdp");

            if (!json_object_has_member(child, "type")) {
                cleanup_and_quit_loop("ERROR: received SDP without 'type'",
                                      PEER_CALL_ERROR);
                goto out;
            }

            sdptype = json_object_get_string_member(child, "type");
            /* In this example, we create the offer and receive one answer by default,
             * but it's possible to comment out the offer creation and wait for an offer
             * instead, so we handle either here.
             *
             * See tests/examples/webrtcbidirectional.c in gst-plugins-bad for another
             * example how to handle offers from peers and reply with answers using webrtcbin. */
            text = json_object_get_string_member(child, "sdp");
            ret = gst_sdp_message_new(&sdp);
            g_assert_cmphex (ret, ==, GST_SDP_OK);
            ret = gst_sdp_message_parse_buffer((guint8 *) text, strlen(text), sdp);
            g_assert_cmphex (ret, ==, GST_SDP_OK);

            if (g_str_equal(sdptype, "answer")) {
                gst_print("Received answer:\n%s\n", text);
                answer = gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_ANSWER,
                                                            sdp);
                g_assert_nonnull (answer);

                /* Set remote description on our pipeline */
                {
                    GstPromise *promise = gst_promise_new();
                    g_signal_emit_by_name(webrtc1_, "set-remote-description", answer,
                                          promise);
                    gst_promise_interrupt(promise);
                    gst_promise_unref(promise);
                }
                app_state_ = PEER_CALL_STARTED;
            } else {
                gst_print("Received offer:\n%s\n", text);
                on_offer_received(sdp);
            }

        } else if (json_object_has_member(object, "ice")) {
            const gchar *candidate;
            gint sdpmlineindex;

            child = json_object_get_object_member(object, "ice");
            candidate = json_object_get_string_member(child, "candidate");
            sdpmlineindex = json_object_get_int_member(child, "sdpMLineIndex");

            /* Add ice candidate sent by remote peer */
            g_signal_emit_by_name(webrtc1_, "add-ice-candidate", sdpmlineindex,
                                  candidate);
        } else {
            gst_printerr("Ignoring unknown JSON message:\n%s\n", text);
        }
        g_object_unref(parser);
    }

    out:
    g_free(text);
}

void WebRTC::on_server_connected(SoupSession *session, GAsyncResult *res,
                    SoupMessage *msg) {
    GError *error = NULL;

    ws_conn_ = soup_session_websocket_connect_finish(session, res, &error);
    if (error) {
        cleanup_and_quit_loop(error->message, SERVER_CONNECTION_ERROR);
        g_error_free(error);
        return;
    }

    g_assert_nonnull (ws_conn_);

    app_state_ = SERVER_CONNECTED;
    gst_print("Connected to signalling server\n");

    g_signal_connect (ws_conn_, "closed", G_CALLBACK(on_server_closed), NULL);
    g_signal_connect (ws_conn_, "message", G_CALLBACK(on_server_message), NULL);

    /* Register with the server so it knows about us and can accept commands */
    register_with_server();
}

/*
 * Connect to the signalling server. This is the entrypoint for everything else.
 */
void WebRTC::connect_to_websocket_server_async(void) {
    SoupLogger *logger;
    SoupMessage *message;
    SoupSession *session;
    const char *https_aliases[] = {"wss", NULL};

    session =
            soup_session_new_with_options(SOUP_SESSION_SSL_STRICT, !disable_ssl_,
                                          SOUP_SESSION_SSL_USE_SYSTEM_CA_FILE, TRUE,
                    //SOUP_SESSION_SSL_CA_FILE, "/etc/ssl/certs/ca-bundle.crt",
                                          SOUP_SESSION_HTTPS_ALIASES, https_aliases, NULL);

    logger = soup_logger_new(SOUP_LOGGER_LOG_BODY, -1);
    soup_session_add_feature(session, SOUP_SESSION_FEATURE (logger));
    g_object_unref(logger);

    message = soup_message_new(SOUP_METHOD_GET, server_url_);

    gst_print("Connecting to server...\n");

    /* Once connected, we will register */
    soup_session_websocket_connect_async(session, message, NULL, NULL, NULL,
                                         (GAsyncReadyCallback) on_server_connected, message);
    app_state_ = SERVER_CONNECTING;
}


WebRTC::WebRTC() {

}

int WebRTC::Start() {
    GOptionContext *context;
    GError *error = NULL;

    setlocale(LC_ALL, "");
    gst_init(nullptr, nullptr);

//    context = g_option_context_new ("- gstreamer webrtc sendrecv demo");
//    g_option_context_add_main_entries (context, entries, NULL);
//    g_option_context_add_group (context, gst_init_get_option_group ());
//    if (!g_option_context_parse (context, &argc, &argv, &error)) {
//        gst_printerr ("Error initializing: %s\n", error->message);
//        return -1;
//    }

//    if (!check_plugins ())
//        return -1;

    if (!peer_id_) {
        gst_printerr ("--peer-id is a required argument\n");
        return -1;
    }

    /* Disable ssl when running a localhost server, because
     * it's probably a test server with a self-signed certificate */
    {
        GstUri *uri = gst_uri_from_string (server_url_);
//        gst_print("uri: %s\n", uri);

//        GstUri *uri = gst_uri_from_string ("wss://webrtc.nirbheek.in:8443");
        if (g_strcmp0 ("localhost", gst_uri_get_host (uri)) == 0 ||
            g_strcmp0 ("127.0.0.1", gst_uri_get_host (uri)) == 0)
            disable_ssl_ = TRUE;
        gst_uri_unref (uri);
    }

    loop_ = g_main_loop_new (NULL, FALSE);

    connect_to_websocket_server_async ();

//    std::thread t = std::thread([&]() {
//        using namespace std::chrono_literals;
//
//        while (1) {
//            std::this_thread::sleep_for(3s);
//
//            g_signal_emit_by_name(send_channel_, "send-string", "{t: 1}");
//
//
//        }
//    });

    g_main_loop_run (loop_);
    g_main_loop_unref (loop_);

    if (pipe1_) {
        gst_element_set_state (GST_ELEMENT (pipe1_), GST_STATE_NULL);
        gst_print ("Pipeline stopped\n");
        gst_object_unref (pipe1_);
    }

    return 0;
}