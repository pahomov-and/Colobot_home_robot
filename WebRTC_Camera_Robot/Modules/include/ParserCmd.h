//
// Created by Andrey Pahomov on 12.03.21.
//

#ifndef PARSERCMD_H
#define PARSERCMD_H

#include "Definitions.h"
#include <map>


class ParserCmd {
public:
    static CMD_t FindCMD(std::string grup, std::string cmd);

    static std::string FindCMD(CMD_t cmd);

    static DATA_INDEX_t FindDataIndex(CMD_t cmd);

private:
    static std::map<CMD_t, DATA_INDEX_t> mapParseData_;
    static std::map<std::string, std::map<std::string, CMD_t>> mapPreparing_;

};

#endif //PARSERCMD_H
