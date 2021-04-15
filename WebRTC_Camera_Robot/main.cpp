#include <iostream>
#include <thread>
#include <string>
#include "WebRTC.h"


WebRTC webRtc;
int main(int argc, char *argv[]) {
//    std::string json = "{\"SET\": {\"POWER\": 0.65, \"DIRECTION\": 0.8}}";
//    std::map<CMD_t, DATA_t> ret_parse_data;
//    Utils::ParseData(json, ret_parse_data);
//
//    std::map<CMD_t, DATA_t> ret_processor;
//    PROCESSOR->Run(ret_parse_data, ret_processor);

    return webRtc.Start();
//    return 0;
}