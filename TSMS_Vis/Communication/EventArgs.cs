using System;

namespace TSMS_Comm
{
    public delegate void CommandSentEventHandler(object sender, EventArgs e);
    public delegate void CommandSendingFailedEventHandler(object sender, EventArgs e);
    public delegate void ConnectionChangedEventHandler(object sender, ConnectionChanged_EventArgs e);
    public delegate void ValueReceivedEventHandler(object sender, TSMS_Value_EventArgs e);
    public delegate void LotDataReceivedEventHandler(object sender, TSMS_LotData_EventArgs e);
    public delegate void PATUpdateReceivedEventHandler(object sender, TSMS_PATUpdate_EventArgs e);
    public delegate void CounterReceivedEventHandler(object sender, TSMS_Counter_EventArgs e);
    public delegate void SetupReceivedEventHandler(object sender, TSMS_Setup_EventArgs e);
    public delegate void MachineSignalsReceivedEventHandler(object sender, TSMS_MachineSignals_EventArgs e);
    public delegate void StatusReceivedEventHandler(object sender, TSMS_Status_EventArgs e);
    public delegate void CompStatusReceivedEventHandler(object sender, TSMS_CompStatus_EventArgs e);
    public delegate void UserMessageReceivedEventHandler(object sender, TSMS_UserMessage_EventArgs e);


    public class ReceiveEventArgs : EventArgs
    {
        private UDPReceiver client;
        public UDPReceiver Client
        {
            get { return client; }
        }
        public ReceiveEventArgs(UDPReceiver cManager)
        {
            client = cManager;
        }
    }

    public class TSMS_Value_EventArgs : EventArgs
    {
        private TSMS_MeasuredValue mv_data;
        public TSMS_MeasuredValue MvData
        {

            get { return mv_data; }
        }
        public TSMS_Value_EventArgs(TSMS_MeasuredValue d)
        {
            mv_data = d;
        }
    }

    public class TSMS_LotData_EventArgs : EventArgs
    {
        private TSMS_LotData data;
        public TSMS_LotData Data
        {
            get { return data; }
        }
        public TSMS_LotData_EventArgs(TSMS_LotData d)
        {
            data = d;
        }
    }

    public class TSMS_PATUpdate_EventArgs : EventArgs
    {
        private TSMS_PATUpdate data;
        public TSMS_PATUpdate Data
        {
            get { return data; }
        }
        public TSMS_PATUpdate_EventArgs(TSMS_PATUpdate d)
        {
            data = d;
        }
    }

    public class TSMS_Counter_EventArgs : EventArgs
    {
        private TSMS_Counter data;
        public TSMS_Counter Data
        {
            get { return data; }
        }
        public TSMS_Counter_EventArgs(TSMS_Counter d)
        {
            data = d;
        }
    }

    public class TSMS_Setup_EventArgs : EventArgs
    {
        private TSMS_Setup data;
        public TSMS_Setup Data
        {
            get { return data; }
        }
        public TSMS_Setup_EventArgs(TSMS_Setup d)
        {
            data = d;
        }
    }

   public class TSMS_MachineSignals_EventArgs : EventArgs
    {
        public TSMS_MachineSignals Data { get; }
        
        public TSMS_MachineSignals_EventArgs(TSMS_MachineSignals d)
        {
            Data = d;
        }
    }

    public class TSMS_Status_EventArgs : EventArgs
    {
        public TSMS_Status Data { get; }

        public TSMS_Status_EventArgs(TSMS_Status d)
        {
            Data = d;
        }
    }

    public class TSMS_CompStatus_EventArgs : EventArgs
    {
        public TSMS_CompStatus Data { get; }

        public TSMS_CompStatus_EventArgs(TSMS_CompStatus d)
        {
            Data = d;
        }
    }

    public class TSMS_UserMessage_EventArgs : EventArgs
    {
        public TSMS_UserMessage Data { get; }

        public TSMS_UserMessage_EventArgs(TSMS_UserMessage d)
        {
            Data = d;
        }
    }

    public class ConnectionChanged_EventArgs : EventArgs
    {
        public bool MasterReady { get; }

        public ConnectionChanged_EventArgs(bool ready)
        {
            MasterReady = ready;
        }
    }

   
}
