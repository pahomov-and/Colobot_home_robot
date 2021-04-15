//
// Created by tymbys on 11.04.2021.
//

#ifndef WEBRTC_CAMERA_WEBRTC_H
#define WEBRTC_CAMERA_WEBRTC_H

#include <iostream>

extern "C"
{
#include <gst/gst.h>
#include <gst/sdp/sdp.h>

#define GST_USE_UNSTABLE_API
#include <gst/webrtc/webrtc.h>

/* For signalling */
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
}

#include <thread>
#include <chrono>

#include "ParserCmd.h"
#include "Processor.h"
#include "Utills.h"

enum AppState
{
    APP_STATE_UNKNOWN = 0,
    APP_STATE_ERROR = 1,          /* generic error */
    SERVER_CONNECTING = 1000,
    SERVER_CONNECTION_ERROR,
    SERVER_CONNECTED,             /* Ready to register */
    SERVER_REGISTERING = 2000,
    SERVER_REGISTRATION_ERROR,
    SERVER_REGISTERED,            /* Ready to call a peer */
    SERVER_CLOSED,                /* server connection closed by us or the server */
    PEER_CONNECTING = 3000,
    PEER_CONNECTION_ERROR,
    PEER_CONNECTED,
    PEER_CALL_NEGOTIATING = 4000,
    PEER_CALL_STARTED,
    PEER_CALL_STOPPING,
    PEER_CALL_STOPPED,
    PEER_CALL_ERROR,
};


class WebRTC {
public:
    WebRTC();
    int Start();


    static gboolean cleanup_and_quit_loop(const gchar *msg, enum AppState state);
    static gchar* get_string_from_json_object(JsonObject *object);
    static void send_ice_candidate_message(GstElement *webrtc G_GNUC_UNUSED, guint mlineindex,
                                           gchar *candidate, gpointer user_data G_GNUC_UNUSED);
    static void send_sdp_to_peer(GstWebRTCSessionDescription *desc);
    static void on_offer_created(GstPromise *promise, gpointer user_data);
    static void on_negotiation_needed(GstElement *element, gpointer user_data);
    static void data_channel_on_error(GObject *dc, gpointer user_data);
    static void data_channel_on_open(GObject *dc, gpointer user_data);
    static void data_channel_on_close(GObject *dc, gpointer user_data);
    static void data_channel_on_message_string(GObject *dc, gchar *str, gpointer user_data);
    static void connect_data_channel_signals(GObject *data_channel);
    static void on_data_channel(GstElement *webrtc, GObject *data_channel,
                                gpointer user_data);
    static void on_ice_gathering_state_notify(GstElement *webrtcbin, GParamSpec *pspec,
                                              gpointer user_data);
    static gboolean start_pipeline(void);
    static gboolean setup_call(void);
    static gboolean register_with_server(void);
    static void on_server_closed(SoupWebsocketConnection *conn G_GNUC_UNUSED,
                                 gpointer user_data G_GNUC_UNUSED);
    static void on_answer_created(GstPromise *promise, gpointer user_data);
    static void on_offer_set(GstPromise *promise, gpointer user_data);
    static void on_offer_received(GstSDPMessage *sdp);
    static void on_server_message(SoupWebsocketConnection *conn, SoupWebsocketDataType type,
                                  GBytes *message, gpointer user_data);
    static void on_server_connected(SoupSession *session, GAsyncResult *res,
                                    SoupMessage *msg);
    static void connect_to_websocket_server_async(void);


private:

    static GMainLoop *loop_;
    static GstElement *pipe1_, *webrtc1_;
    static GObject *send_channel_, *receive_channel_;

    static SoupWebsocketConnection *ws_conn_;
    static AppState app_state_;
    static const gchar *peer_id_;
    static const gchar *server_url_;
    static gboolean disable_ssl_;
    static gboolean remote_is_offerer_;

};


#endif //WEBRTC_CAMERA_WEBRTC_H
