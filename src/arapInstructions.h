#pragma once



enum ARAP_MODE {
    STANDBY = 0,
    OPERATION = 1,
    SYNCHRONIZATION = 2,
    OPERATIONANDSYNCHRONIZATION = 3,
    EXECUTION = 20,
    EMERGENCYSTOP = 30
};

struct ARAP_COORDINATE {
    int x;
    int y;
    int z;
    int q1;
    int q2;
    int q3;
    int q4;
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
    ARAP_COORDINATE location;
};

struct ARAP_PROGRAM {
    unsigned char * data;
};

