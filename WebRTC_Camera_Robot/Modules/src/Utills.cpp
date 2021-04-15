//
// Created by tymbys on 14.04.2021.
//

#include "Utills.h"

//https://docs.tizen.org/development/sample/native/Web/%28Tutorial%29_Json-Glib/src/json_manager.c

static void parse_node(JsonNode *node, const char *key, gpointer data);
static void object_member_cb(JsonObject *object, const gchar *member_name, JsonNode *member_node,
                      gpointer user_data);
static void array_element_cb(JsonArray *array, guint index_, JsonNode *element_node,
                             gpointer user_data);



static void object_member_cb(JsonObject *object, const gchar *member_name, JsonNode *member_node,
                             gpointer user_data)
{
    parse_node(member_node, member_name, user_data);
}

static void array_element_cb(JsonArray *array, guint index_, JsonNode *element_node,
                             gpointer user_data)
{
    parse_node(element_node, NULL, user_data);
}

static void parse_node(JsonNode *node, const char *key, gpointer data) {
    if (json_node_get_node_type(node) != JSON_NODE_VALUE &&
    json_node_get_node_type(node) != JSON_NODE_OBJECT) {
        return;
    }


    /* Create structure instance for node */
//    associated_data_s *new_data = malloc(sizeof(associated_data_s));
//    memset(new_data, 0, sizeof(associated_data_s));

//    draw_member(new_data, data);


//    if (key != NULL)
//        elm_object_text_set(new_data->my_key_entry, key);

    /* Get type of node */
    JsonNodeType type = json_node_get_node_type(node);

    switch (type) {
        case JSON_NODE_OBJECT: {
            /* Draw a form for an object */
//            draw_object_form(new_data);

            JsonObject *object = json_node_get_object(node);

            /* Iterate through object members */
//            json_object_foreach_member(object, object_member_cb, (gpointer) new_data);
            json_object_foreach_member(object, object_member_cb, (gpointer) data);
            break;
        }

        case JSON_NODE_ARRAY: {
            /* Draw a form for an array */
//            draw_array_form(new_data);

            JsonArray *array = json_node_get_array(node);

            /* Iterate through array elements */
//            json_array_foreach_element(array, array_element_cb, (gpointer) new_data);
            json_array_foreach_element(array, array_element_cb, (gpointer) data);
            break;
        }

        case JSON_NODE_VALUE: {
            /* Draw a form for a value */
//            draw_value_form(new_data);

            /* Get value of node and check its type */
            GValue gvalue = {0,};
            json_node_get_value(node, &gvalue);

            /* Display value in entry widget */
            switch (json_node_get_value_type(node)) {
                case G_TYPE_STRING: {
                    const char *value = g_value_get_string(&gvalue);
//                    elm_object_text_set(new_data->my_value_entry, value);
                    break;
                }

                case G_TYPE_INT64: {
                    char value_tmp[20];
                    gint value64 = g_value_get_int64(&gvalue);
//                    snprintf(value_tmp, 10, "%d", value64);
//                    elm_object_text_set(new_data->my_value_entry, value_tmp);
                    break;
                }

                case G_TYPE_DOUBLE: {
                    gdouble d_value = g_value_get_double(&gvalue);
//                    snprintf(value_tmp, 10, "%f", d_value);
//                    elm_object_text_set(new_data->my_value_entry, value_tmp);
                    break;
                }

                case G_TYPE_BOOLEAN: {
                    gboolean b_value = g_value_get_boolean(&gvalue);
//                    snprintf(value_tmp, 10, "%s", b_value ? "TRUE" : "FALSE");
//                    elm_object_text_set(new_data->my_value_entry, value_tmp);
                    break;
                }
            }

            /* Free the value */
            g_value_unset(&gvalue);
            break;
        }
        default: {
        }

    }
}

namespace Utils {

    int ParseData(std::string &json_str, std::map<CMD_t, DATA_t> &ret) {
        JsonNode *root;
        JsonObject *object, *child;

        if (json_str.empty())
            return -1;

//        gchar *text = static_cast<const char*>(json_str.c_str());
        gchar *text = const_cast<gchar *>(json_str.c_str());

        JsonParser *parser = json_parser_new();
        if (!json_parser_load_from_data(parser, text, -1, NULL)) {
            LOG_ERROR("Unknown message '", text, "', ignoring");
            g_object_unref(parser);
            return -1;
        }

        root = json_parser_get_root(parser);
        if (!JSON_NODE_HOLDS_OBJECT (root)) {
            LOG_ERROR("Unknown json message '", text, "', ignoring");
            g_object_unref(parser);
            return -1;
        }

        object = json_node_get_object(root);

        /****/
        //json_object_foreach_member(object, object_member_cb, (gpointer) &ret);
        json_object_foreach_member(object, [](JsonObject *object,
                                              const gchar *member_name,
                                              JsonNode *member_node,
                                              gpointer user_data) {

            std::string grup_str(member_name);
            std::transform(grup_str.begin(), grup_str.end(), grup_str.begin(), ::toupper);

            if (grup_str == "SET") {

                JsonNodeType type = json_node_get_node_type(member_node);

                if (type == JSON_NODE_OBJECT) {

                    JsonObject *object = json_node_get_object(member_node);

                    /****/
                    json_object_foreach_member(object, [](JsonObject *object,
                                                          const gchar *member_name,
                                                          JsonNode *member_node,
                                                          gpointer user_data) {


                        std::string cmd_str(member_name);
                        std::transform(cmd_str.begin(), cmd_str.end(), cmd_str.begin(), ::toupper);

                        CMD_t cmd_cur = ParserCmd::FindCMD("SET", cmd_str);

//                        DATA_INDEX_t data_index = ParserCmd::FindDataIndex(cmd_cur);

                        DATA_t data = 0;
                        GValue gvalue = {0,};

                        JsonNodeType type = json_node_get_node_type(member_node);

                        if (type == JSON_NODE_VALUE) {
                            json_node_get_value(member_node, &gvalue);
                            DATA_INDEX_t data_index = ParserCmd::FindDataIndex(cmd_cur);


                            switch (data_index) {
                                case DATA_INDEX_DOUBLE: {
                                    switch (json_node_get_value_type(member_node)) {
                                        case G_TYPE_INT64: {
                                            data = (double )g_value_get_int64(&gvalue);
                                            break;
                                        }
                                        case G_TYPE_DOUBLE: {
                                            data = g_value_get_double(&gvalue);
                                            break;
                                        }
                                    }

                                    break;
                                }
                                case DATA_INDEX_NULL:
                                    break;
                            }

                            std::map<CMD_t, DATA_t> *ret = (std::map<CMD_t, DATA_t> *) user_data;

                            ret->insert(std::pair(cmd_cur, data));
                        }
                    }, user_data);
                }
            }
        }, (gpointer) &ret);


        return 0;
    }
}