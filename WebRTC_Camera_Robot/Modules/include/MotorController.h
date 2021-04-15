//
// Created by tymbys on 26.03.2021.
//

#ifndef WEBRTC_CAMERA_MOTORCONTROLLER_H
#define WEBRTC_CAMERA_MOTORCONTROLLER_H
#include <mutex>
#include <memory>
#include <atomic>

enum STATUS_JOYSTICK {
    JOYSTICK_NULL = 0,
    JOYSTICK_X = 1,
    JOYSTICK_Y= 2,

};

typedef struct {
    double x;
    double y;
    uint8_t flag;
} JOYSTICK_t;

/**
 * cmd:
 *
 * [0] 0xAx - motor right,left on
 * 		1010 xx01b - motor right run left
 * 		1010 xx10b - motor right run right
 * 		1010 xx00b - motor right run stop
 * 		1010 xx11b - motor right run stop
 *
 * 		1010 01xxb - motor left run left
 * 		1010 10xxb - motor left run right
 * 		1010 00xxb - motor right run stop
 * 		1010 00xxb - motor right run stop
 *
 * [0] 0xBx - motor right set power
 *		1011 xxxxb - xx 0000..1111
 *
 * [0] 0xCx - motor left set power
 *		1100 xxxxb - xx 0000..1111
 *
 *
 *
 */

enum BIT_MOTOR {
    MOTOR_RIGHT_RUN_LEFT = 0xA1,
    MOTOR_RIGHT_RUN_RIGHT = 0xA2,
    MOTOR_RIGHT_RUN_STOP = 0xA3,

    MOTOR_LEFT_RUN_LEFT = 0xA4,
    MOTOR_LEFT_RUN_RIGHT = 0xA8,
    MOTOR_LEFT_RUN_STOP = 0xAC,

};

class MotorController {
public:
    static MotorController *get() {

        MotorController *sin = instance_.load(std::memory_order_acquire);

        if (!sin) {
            std::lock_guard<std::mutex> myLock(mutexConfigs_);
            sin = instance_.load(std::memory_order_relaxed);
            if (!sin) {
                sin = new MotorController();
                instance_.store(sin, std::memory_order_release);
            }
        }
        return sin;
    }


    void SetMotors(JOYSTICK_t motor);

private:
    static std::atomic<MotorController *> instance_;
    static std::mutex mutexConfigs_;
};

#define MOTOR MotorController::get()

#endif //WEBRTC_CAMERA_MOTORCONTROLLER_H
