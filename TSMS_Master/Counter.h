#pragma once
#include "const.h"

struct MachineCounter
{
    MachineCounter();
    void Reset();
    void Add(MachineCounter &newCnt);

    unsigned int total_input = 0;
    unsigned int total_good = 0;

    unsigned int TRB[tsms_config::NUM_COUNTER_TRB] = { 0 };
    unsigned int TRV[tsms_config::NUM_COUNTER_VRB] = { 0 };

    unsigned int good_bin = 0;
    unsigned int last_bin = 0;
};

struct TSMSCounter
{
    unsigned int input_parts = 0;
    unsigned int good_parts = 0;
    unsigned int res_pass[tsms_config::NUM_MEASUREMENTS] = { 0 };
    unsigned int res_low[tsms_config::NUM_MEASUREMENTS] = { 0 };
    unsigned int res_high[tsms_config::NUM_MEASUREMENTS] = { 0 };
    unsigned int res_fail[tsms_config::NUM_MEASUREMENTS] = { 0 };
    
    void Reset();
    void Copy(TSMSCounter& newCnt);
};
