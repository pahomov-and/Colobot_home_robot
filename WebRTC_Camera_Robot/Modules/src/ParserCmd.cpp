//
// Created by Andrey Pahomov on 12.03.21.
//

#include "ParserCmd.h"

//GET_POWER,
//GET_DIRECTION,
std::map<CMD_t, DATA_INDEX_t> ParserCmd::mapParseData_ = {
        {SET_JOYSTICK_X, DATA_INDEX_DOUBLE},
        {SET_JOYSTICK_Y, DATA_INDEX_DOUBLE},

        {GET_JOYSTICK_X, DATA_INDEX_DOUBLE},
        {GET_JOYSTICK_Y, DATA_INDEX_DOUBLE},

};


std::map<std::string, std::map<std::string, CMD_t>> ParserCmd::mapPreparing_ = {
        {
                "SET",
                {
                        {"JOYSTICK_X", SET_JOYSTICK_X},
                        {"JOYSTICK_Y", SET_JOYSTICK_Y},
                }
        },
        {
                "GET",
                {
                        {"JOYSTICK_X", GET_JOYSTICK_X},
                        {"JOYSTICK_Y", GET_JOYSTICK_Y},
                }
        }
};


CMD_t ParserCmd::FindCMD(std::string grup, std::string cmd) {

    for (auto &dot: ParserCmd::mapPreparing_[grup]) {
        if (dot.first == cmd) {
            return dot.second;
        }
    }

    return CMD_NULL;
}

std::string ParserCmd::FindCMD(CMD_t cmd) {
    for (auto &dot: ParserCmd::mapPreparing_["GET"]) {
        if (dot.second == cmd) {
            return dot.first;
        }
    }

    return "";
}

DATA_INDEX_t ParserCmd::FindDataIndex(CMD_t cmd) {

    DATA_INDEX_t ret = ParserCmd::mapParseData_[cmd];
    if (ret) {

        return ret;
    }

    return DATA_INDEX_NULL;
}
