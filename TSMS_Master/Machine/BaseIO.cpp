#include "DAQmxIO.h"
#include "../NI/NIDAQmx.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <stdexcept>

#define PRINT_LOT_SIGNALS 0

namespace io
{
#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) Error(); else

	static TaskHandle	task_IN = 0;
	static TaskHandle	task_OUT = 0;
#define BITS_TO_READ 32
#define BITS_TO_WRITE 32
	static uInt32	numLines;
	static uInt8	data_in[BITS_TO_READ] = { 0 };
	static uInt8	data_in_cached[BITS_TO_READ];
	static uInt8	data_out_cached[BITS_TO_WRITE] = { 0 };
	static func_t   registered_functions[BITS_TO_READ] = { 0 };

	void Cleanup(void);
	void Error();

	int32 CVICALLBACK ChangeDetectionCallback(TaskHandle taskHandle, int32 signalID, void* callbackData)
	{
		int32   error = 0;
		int32   numRead = 0;
		uInt32  i = 0;

#if PRINT_LOT_SIGNALS
		char    buff[512], * buffPtr;
		char    errBuff[2048] = { '\0' };
#endif
		if (taskHandle)
		{
			// DAQmx Read
			DAQmxErrChk(DAQmxReadDigitalLines(taskHandle, 1, 10.0, DAQmx_Val_GroupByScanNumber, data_in, numLines, &numRead, NULL, NULL));

			if (numRead)
			{
#if PRINT_LOT_SIGNALS
				buffPtr = buff;
				for (i = 0; i < numLines; ++i)
				{
					sprintf(buffPtr, "%d", data[i]);
					buffPtr++;
				}

				strcat(buff, "    ");
				buffPtr = buff + strlen(buff);
#endif
				for (i = 0; i < numLines; ++i)
				{
#if PRINT_LOT_SIGNALS
					sprintf(buffPtr, "%c", data[i] != data_in_cached[i] ? 'X' : '-');
					buffPtr++;
#endif
					if (data_in[i] != data_in_cached[i])
						if (registered_functions[i] != 0)
							registered_functions[i]();		// call registered function

					data_in_cached[i] = data_in[i];
				}
#if PRINT_LOT_SIGNALS
				puts(buff);
				fflush(stdout);
#endif
			}
		}
		return 0;
	}

	void ReadWithoutChangeDetection()
	{
		int32 numRead = 0;
		int32 error = DAQmxReadDigitalLines(task_IN, 1, 10.0, DAQmx_Val_GroupByScanNumber, data_in, numLines, &numRead, NULL, NULL);
		if (DAQmxFailed(error))
			Error();
		for (unsigned int i = 0; i < numLines; ++i)
		{
			data_in_cached[i] = data_in[i];
		}
	}

	void Init_Input(bool withChangeDetection)
	{
		int32 error = 0;
		DAQmxErrChk(DAQmxCreateTask("", &task_IN));
		DAQmxErrChk(DAQmxCreateDIChan(task_IN, "Dev1/line0:31", "", DAQmx_Val_ChanPerLine));
		if (withChangeDetection)
		{
			DAQmxErrChk(DAQmxCfgChangeDetectionTiming(task_IN, "Dev1/line0:31", "Dev1/line0:31", DAQmx_Val_ContSamps, 1));
			DAQmxErrChk(DAQmxRegisterSignalEvent(task_IN, DAQmx_Val_ChangeDetectionEvent, 0, ChangeDetectionCallback, NULL));
		}
		DAQmxErrChk(DAQmxGetTaskNumChans(task_IN, &numLines));
		DAQmxErrChk(DAQmxStartTask(task_IN));
	}

	void Init_Ouput()
	{
		int32 error = 0;
		DAQmxErrChk(DAQmxCreateTask("", &task_OUT));
		DAQmxErrChk(DAQmxCreateDOChan(task_OUT, "Dev1/port4/line0:7", "", DAQmx_Val_ChanForAllLines));
		DAQmxErrChk(DAQmxCreateDOChan(task_OUT, "Dev1/port5/line0:7", "", DAQmx_Val_ChanForAllLines));
		DAQmxErrChk(DAQmxCreateDOChan(task_OUT, "Dev1/port6/line0:7", "", DAQmx_Val_ChanForAllLines));
		DAQmxErrChk(DAQmxCreateDOChan(task_OUT, "Dev1/port7/line0:7", "", DAQmx_Val_ChanForAllLines));
		DAQmxErrChk(DAQmxStartTask(task_OUT));
	}

	void Write()
	{
		int32 error = DAQmxWriteDigitalLines(task_OUT, 1, 1, 10.0, DAQmx_Val_GroupByChannel, data_out_cached, NULL, NULL);
		if (DAQmxFailed(error))
			Error();
	}

	void Error()
	{
		char errBuff[2048] = { '\0' };
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
		Cleanup();
		std::string errorMsg = "DAQmx Error: " + std::string(errBuff);
		throw std::runtime_error(errorMsg);
	}

	void Cleanup(void)
	{
		if (task_IN != 0)
		{
			DAQmxStopTask(task_IN);
			DAQmxClearTask(task_IN);
			task_IN = 0;
		}
		if (task_OUT != 0)
		{
			DAQmxStopTask(task_OUT);
			DAQmxClearTask(task_OUT);
			task_OUT = 0;
		}
	}

	DAQmxIO::DAQmxIO()
	{
		isInit = false;
	}

	DAQmxIO::~DAQmxIO()
	{
		CloseIO();
	}

	void DAQmxIO::RegisterFunction(InputPins pin, func_t func)
	{
		registered_functions[pin] = func;
	}

	bool DAQmxIO::GetPin(InputPins pin)
	{
		return data_in[pin];
	}

	void DAQmxIO::SetPin(OutputPins pin)
	{
		if (!isInit)
			return;

		data_out_cached[pin] = 1;
		Write();
	}

	void DAQmxIO::ResetPin(OutputPins pin)
	{
		if (!isInit)
			return;

		data_out_cached[pin] = 0;
		Write();

	}

	void DAQmxIO::ResetAllPins()
	{
		if (!isInit)
			return;

		memset(&data_out_cached, 0, BITS_TO_WRITE * sizeof(uInt8));
		Write();

	}

	void DAQmxIO::PrintDebugOutputPins()
	{
		char buff[512];
		char* buffPtr = buff;
		for (int i = 0; i < BITS_TO_WRITE; ++i)
		{
			if (i % 8 == 0)
			{
				sprintf(buffPtr, " ");
				buffPtr++;
			}
			sprintf(buffPtr, "%c", data_out_cached[i] ? '|' : '_');
			buffPtr++;
		}
		puts(buff);
		fflush(stdout);
	}

	void DAQmxIO::InitIO()
	{
		if (isInit)
			CloseIO();

		Init_Input(false);
		ReadWithoutChangeDetection();	// load current pin status in cache
		Cleanup();
		Init_Input(true);	// activate change detection
		Init_Ouput();
		isInit = true;

		printf("Digital IO initialized\n");
	}

	void DAQmxIO::CloseIO()
	{
		if (isInit)
		{
			ResetAllPins();
			Cleanup();
			isInit = false;
		}
	}
}