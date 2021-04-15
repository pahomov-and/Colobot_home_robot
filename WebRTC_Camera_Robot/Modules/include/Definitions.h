//
// Created by Andrey Pahomov on 11.03.21.
//

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <vector>
#include <variant>
#include <functional>

enum CMD_t : size_t {

    // Grup set
    SET_JOYSTICK_X,
    SET_JOYSTICK_Y,
//    SET_TRACING,
//    SET_DETECTION_START,
//    SET_DETECTION_STOP,


    // Grup get
    GET_JOYSTICK_X,
    GET_JOYSTICK_Y,
//    GET_DETECTION_STATUS,
//    GET_DETECTION_OBJECTS,

    CMD_NULL
};

enum DATA_INDEX_t : size_t {
    DATA_INDEX_SIZE_T,
    DATA_INDEX_INT,
    DATA_INDEX_DOUBLE,
    DATA_INDEX_STRING,
    DATA_INDEX_VECTOR_UINT16,

    DATA_INDEX_NULL

};

using DATA_t = std::variant<  // DATA_INDEX_t
        size_t,
        int,
        double,
        std::string,
        std::vector<uint16_t>
>;

using CALL_t = std::function<void(DATA_t, DATA_t&)>;


#endif //DEFINITIONS_H
