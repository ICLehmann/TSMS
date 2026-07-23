#pragma once

namespace io
{
#define BITS_TO_READ 32
#define BITS_TO_WRITE 32

    typedef void (*func_t)();

    enum InputPins
    {
        // Port 0
        IN00_MACHINE_EMPTY = 0,
        IN01_SYSTEM_READY,
        IN02_LOT_START,
        IN03_LOT_END,
        IN04_LOT_PAUSE,
        IN05_LOT_DISCARD,
        IN06_E_COMPENSATION,
        IN07_E_DUMMY,

        // Port 1
        IN10_RFID_TRIGGER,
        IN11_ISOLATION_TRIGGER,
        IN12_IMPED_INDUCT_TRIGGER,
        IN13_RESERVE,
        IN14_CAPACITY_TRIGGER,
        IN15_RESISTANCE_TRIGGER,
        IN16_RESERVE,
        IN17_RESERVE,

        // Port 2
        IN20_LOT_IN_PROGRESS,
        IN21_INIT_TRIGGER,
        IN22_SHIFT_TRIGGER,
        IN23_RESERVE,
        IN24_RESERVE,
        IN25_RESERVE,
        IN26_RESERVE,
        IN27_RESERVE,

        // Port 3
        IN30_RESERVE,
        IN31_RESERVE,
        IN32_RESERVE,
        IN33_RESERVE,
        IN34_RESERVE,
        IN35_RESERVE,
        IN36_RESERVE,
        IN37_RESERVE
    };

    enum OutputPins
    {
        // Port 4
        OUT40_SYSTEM_READY,
        OUT41_LOT_IN_PROGRESS,
        OUT42_COMPENSATION_IN_PROGRESS,
        OUT43_E_DUMMY_IN_PROGRESS,
        OUT44_SHIFT_TRIGGER,
        OUT45_RESERVE,
        OUT46_RFID_PASS_FAIL,
        OUT47_INIT,

        // Port 5
        OUT50_IMPEDANCE_PASS,
        OUT51_IMPEDANCE_FAIL_LIMIT_HIGH,
        OUT52_IMPEDANCE_FAIL_LIMIT_LOW,
        OUT53_IMPEDANCE_FAIL_SYSTEM,
        OUT54_CAPACITY_PASS,
        OUT55_CAPACITY_FAIL_LIMIT_HIGH,
        OUT56_CAPACITY_FAIL_LIMIT_LOW,
        OUT57_CAPACITY_FAIL_SYSTEM,

        // Port 6
        OUT60_ISOLATION_PASS,
        OUT61_ISOLATION_FAIL_LIMIT_HIGH,
        OUT62_ISOLATION_FAIL_LIMIT_LOW,
        OUT63_ISOLATION_FAIL_SYSTEM,
        OUT64_INDUCTANCE_PASS,
        OUT65_INDUCTANCE_FAIL_LIMIT_HIGH,
        OUT66_INDUCTANCE_FAIL_LIMIT_LOW,
        OUT67_INDUCTANCE_FAIL_SYSTEM,

        // Port 7
        OUT70_RESISTANCE_R1_PASS,
        OUT71_RESISTANCE_R1_FAIL_LIMIT_HIGH,
        OUT72_RESISTANCE_R1_FAIL_LIMIT_LOW,
        OUT73_RESISTANCE_R1_FAIL_SYSTEM,
        OUT74_RESISTANCE_R2_PASS,
        OUT75_RESISTANCE_R2_FAIL_LIMIT_HIGH,
        OUT76_RESISTANCE_R2_FAIL_LIMIT_LOW,
        OUT77_RESISTANCE_R2_FAIL_SYSTEM
    };

    class BaseIO
    {
    private:
        bool isInit;

    public:

        virtual bool GetPin(InputPins pin) = 0;
        virtual void RegisterFunction(InputPins pin, func_t func) = 0;
        virtual void SetPin(OutputPins pin) = 0;
        virtual void ResetPin(OutputPins pin) = 0;
        virtual void ResetAllPins() = 0;
        virtual void PrintDebugOutputPins() = 0;
        virtual void InitIO() = 0;
        virtual void CloseIO() = 0;

		virtual void SimulateCompensation() = 0;
		virtual void SimulateDummyTest() = 0;
		virtual void SimulateNewLOT() = 0;
		virtual void StopSimulation() = 0;
	};
}