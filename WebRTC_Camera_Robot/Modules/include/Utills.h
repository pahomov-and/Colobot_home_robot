//
// Created by tymbys on 14.04.2021.
//

#ifndef WEBRTC_CAMERA_UTILLS_H
#define WEBRTC_CAMERA_UTILLS_H

#include <map>
#include <string>

#include <json-glib/json-glib.h>

#include "Definitions.h"
#include "ParserCmd.h"
#include "LOG.h"


namespace Utils {
    int ParseData(std::string &json_str, std::map<CMD_t, DATA_t> &ret);
}
#endif //WEBRTC_CAMERA_UTILLS_H
