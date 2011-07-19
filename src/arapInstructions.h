#pragma once

enum ARAP_MODE {
    STANDBY = 0,
    OPERATION = 1,
    EXECUTION = 2,
    EMERGENCYSTOP = 3
};

struct ARAP_STATUS {
    int program_number;
    int instruction_number;
    int actual_tcp;
    int actual_frame;
    bool local; //oppsite of remote
    bool interrupt; //permitted or not permitted
    bool programming_unit; //connected or not connected
    bool key; //false = auto | true = auto or test 100%
    ARAP_MODE mode;
};