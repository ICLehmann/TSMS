#pragma once
#include "TestConfig.h"
#include "Counter.h"

struct PATConfig
{
    bool uselastlot;
    int mode;
    int samplecnt;
    int ll_sigma;
    int ul_sigma;
};

namespace lot
{
    enum eLotState { UNKNOWN, STARTED, PAUSED, FINISHED };
    static const std::string LotStateNames[] = { "UNKNOWN", "STARTED", "PAUSED", "FINISHED" };     // used for database
    static const unsigned int NUM_STATES = 5;
    static std::string GetStateAsString(eLotState state) { return LotStateNames[static_cast<eLotState>(state)]; };

    struct LotHeader
    {
        long long lot_nr = 0;
        std::string test_plan;
        std::string line_id;
        std::string operator_id;
        bool is_dummy = false;
    };

    class LOT_Data
    {
  
    public:
        LotHeader header;
        eLotState lot_state;
        PATConfig pat_config;

        std::string testing_machine;
        std::string laser_mark;
        std::string lot_start;
        std::string lot_stop;

        TestConfig test_config;
        MachineCounter machine_cnt;
        TSMSCounter tsms_cnt;
        
        LOT_Data();
        void Reset();

        void SetHeader(LotHeader newHeader);
        void SetStartTimeToNow();
        void SetStopTimeToNow();
        void CreateNewLaserMark();
        void SetPATConfig(PATConfig newPATConfig);
    };
}