//
// Created by tymbys on 26.03.2021.
//

#include "MotorController.h"
#include "LOG.h"

std::atomic<MotorController *> MotorController::instance_;
std::mutex MotorController::mutexConfigs_;

void MotorController::SetMotors(JOYSTICK_t motor) {
    const double threshold = 0.5f;
    uint8_t motor_data = 0;

//    LOG_INFO("x: ", motor.x, "\ty: ",motor.y);

    if (motor.y < -threshold && motor.x > -threshold && motor.x < threshold) {
        LOG_INFO("L+, R+");
        motor_data = MOTOR_LEFT_RUN_RIGHT | MOTOR_RIGHT_RUN_RIGHT;
    } else if (motor.x > threshold ) {
        LOG_INFO("L+, R-");
        motor_data = MOTOR_LEFT_RUN_RIGHT | MOTOR_RIGHT_RUN_LEFT;
    }  if (motor.y > threshold && motor.x > -threshold && motor.x < threshold) {
        LOG_INFO("L-, R-");
        motor_data = MOTOR_LEFT_RUN_LEFT | MOTOR_RIGHT_RUN_LEFT;
    } else if (motor.x < -threshold ) {
        LOG_INFO("L-, R+");
        motor_data = MOTOR_LEFT_RUN_LEFT | MOTOR_RIGHT_RUN_RIGHT;
    }

    if (motor_data > 0) {
        LOG_INFO("motor_data: 0x" , std::hex, (int)motor_data, std::dec);

    }

//    LOG_INFO("x: ", motor.x, "\ty: ",motor.y);

}