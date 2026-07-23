#pragma once
#include "BaseIO.h"
namespace io
{
    class DAQmxIO : public BaseIO
    {
    private:
        bool isInit;

    public:
        DAQmxIO();
        ~DAQmxIO();

        bool GetPin(InputPins pin);
        void RegisterFunction(InputPins pin, func_t func);
        void SetPin(OutputPins pin);
        void ResetPin(OutputPins pin);
        void ResetAllPins();
        void PrintDebugOutputPins();
        void InitIO();
        void CloseIO();

        void SimulateCompensation() {};
        void SimulateDummyTest() {};
        void SimulateNewLOT() {};
        void StopSimulation() {};
    };
}