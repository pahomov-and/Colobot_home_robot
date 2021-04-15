//
// Created by Andrey Pahomov on 11.03.21.
//

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "Definitions.h"
#include "MotorController.h"
#include "LOG.h"

#include <map>
#include <queue>
#include <mutex>
#include <atomic>

enum STATUS_PROCESSOR {
    IS_OK = 0,
    IS_BUSY = 1,

    IS_ERROR_1= 16,
    IS_ERROR_2 = 32,
    IS_ERROR,
    // Commands after sending responses to the client
    NEED_SET_MOTOR
};



class Processor {
public:

    static Processor *get() {

        Processor *sin = instance_.load(std::memory_order_acquire);

        if (!sin) {
            std::lock_guard<std::mutex> myLock(mutexConfigs_);
            sin = instance_.load(std::memory_order_relaxed);
            if (!sin) {
                sin = new Processor();
                instance_.store(sin, std::memory_order_release);
            }
        }
        return sin;
    }


    uint32_t GetStatus();
    void CleanConfigMotor();

    void SetStates(STATUS_PROCESSOR state);

    void ResetStates(STATUS_PROCESSOR state);

    void Processing();

    void Run(std::map<CMD_t, DATA_t> &src, std::map<CMD_t, DATA_t> &ret);

    void RunDeferredCMD();

    static void PROCESSING_SET_JOYSTICK_X(DATA_t arg, DATA_t &ret);
    static void PROCESSING_SET_JOYSTICK_Y(DATA_t arg, DATA_t &ret);
    static void PROCESSING_GET_JOYSTICK_X(DATA_t arg, DATA_t &ret);
    static void PROCESSING_GET_JOYSTICK_Y(DATA_t arg, DATA_t &ret);

private:
    static const std::map<CMD_t, CALL_t> mapCalls_;

    static std::atomic<Processor *> instance_;
    static std::mutex mutexConfigs_;

    static std::atomic<uint32_t> status_processor_;
    static JOYSTICK_t prepare_joystick_;

    static std::queue<STATUS_PROCESSOR> q_after_sending_cmd_;


    Processor()= default;
    ~Processor()= default;
    Processor(const Processor&)= delete;
    Processor& operator=(const Processor&)= delete;
};

#define PROCESSOR Processor::get()

#endif //PROCESSOR_H
