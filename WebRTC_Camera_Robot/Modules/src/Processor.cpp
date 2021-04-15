//
// Created by Andrey Pahomov on 11.03.21.
//

#include <vector>
#include <variant>
#include <string>
#include <iomanip>
#include <future>
#include <iostream>

#include "Processor.h"

std::atomic<Processor *> Processor::instance_;
std::mutex Processor::mutexConfigs_;

JOYSTICK_t Processor::prepare_joystick_;
std::queue<STATUS_PROCESSOR> Processor::q_after_sending_cmd_;

// STATUS_PROCESSOR as uint32_t
std::atomic<uint32_t> Processor::status_processor_ = IS_OK;


const std::map<CMD_t, CALL_t> Processor::mapCalls_ = {
        {SET_JOYSTICK_X,     Processor::PROCESSING_SET_JOYSTICK_X},
        {SET_JOYSTICK_Y, Processor::PROCESSING_SET_JOYSTICK_Y},

        {GET_JOYSTICK_X,     Processor::PROCESSING_GET_JOYSTICK_X},
        {GET_JOYSTICK_Y, Processor::PROCESSING_GET_JOYSTICK_Y},
};

uint32_t Processor::GetStatus() {
    return status_processor_;
}

void Processor::CleanConfigMotor() {
    prepare_joystick_.x = 0;
    prepare_joystick_.y = 0;
    prepare_joystick_.flag = JOYSTICK_NULL;
}

void Processor::SetStates(STATUS_PROCESSOR state) {
    status_processor_ |= state;
    Processing();
}

void Processor::ResetStates(STATUS_PROCESSOR state) {
    status_processor_ &= ~state;
    Processing();
}

void Processor::Processing() {
}

void Processor::Run(std::map<CMD_t, DATA_t> &src, std::map<CMD_t, DATA_t> &ret) {
    SetStates(IS_BUSY);

    for (auto &item: src) {

        if (mapCalls_.count(item.first)) {
            DATA_t ret_node;
            mapCalls_.at(item.first)(item.second, ret_node);
            ret.insert(std::pair(item.first, ret_node));
        }

    }

    ResetStates(IS_BUSY);
}

void Processor::RunDeferredCMD() {
    while (!q_after_sending_cmd_.empty()) {

        switch (q_after_sending_cmd_.front()) {
            case NEED_SET_MOTOR:
                if (prepare_joystick_.flag >=  (JOYSTICK_X | JOYSTICK_Y) )  {
//                    LOG_INFO("Set motor");
                    MOTOR->SetMotors(prepare_joystick_);
                }
                break;
            default:
                break;
        }

        q_after_sending_cmd_.pop();
    }
}


void Processor::PROCESSING_SET_JOYSTICK_X(DATA_t arg, DATA_t &ret) {
    double x = std::get<double>(arg);
//    LOG_INFO(x);

    prepare_joystick_.x = x;
    prepare_joystick_.flag |= JOYSTICK_X;

    if (prepare_joystick_.flag >=  (JOYSTICK_X | JOYSTICK_Y) ) {
        q_after_sending_cmd_.push(NEED_SET_MOTOR);
    }
}

void Processor::PROCESSING_SET_JOYSTICK_Y(DATA_t arg, DATA_t &ret) {
    double y = std::get<double>(arg);
//    LOG_INFO(y);

    prepare_joystick_.y = y;
    prepare_joystick_.flag |= JOYSTICK_Y;

    if (prepare_joystick_.flag >=  (JOYSTICK_X | JOYSTICK_Y) ) {
        q_after_sending_cmd_.push(NEED_SET_MOTOR);
    }
}

void Processor::PROCESSING_GET_JOYSTICK_X(DATA_t arg, DATA_t &ret) {
    LOG_INFO();
}

void Processor::PROCESSING_GET_JOYSTICK_Y(DATA_t arg, DATA_t &ret) {
    LOG_INFO();
}
