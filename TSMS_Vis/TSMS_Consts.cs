namespace TSMS_Vis
{
    public class TSMS_Consts
    {
        public const string INI_FILE = "tsms.ini";

        public const string IP_ADR = "127.0.0.1";
        public const int RECV_PORT = 8500;
        public const int SEND_PORT = 8501;

        public const string IP_ADR_PRINTER = "192.168.0.20";
        public const int PORT_PRINTER = 9100;
        public const string LABEL_TEMPLATE_FILE = "LabelPrinter.txt";

        public const int TELEGRAM_CHAR_LENGTH = 20;      // string length in udp telegrams
        public enum eTelMasterToUI
        {
            TEL_SETUP = 32, TEL_STATUS, TEL_LOT, TEL_COUNTER, TEL_MEAS_VALUE,
            TEL_MACHINE, TEL_COMP_STATUS, TEL_USER_MSG, TEL_PAT_UPDATE
        };	// must be the same like in master

        public enum eTelUItoMaster { TEL_UI_IS_STARTED = 32, TEL_UI_INIT_RS232, TEL_UI_INIT_DB, TEL_UI_INIT_DEVICES, TEL_UI_INIT_IO, TEL_UI_RESTART_PAT };	// must be the same like in master
        public const int RECEIVE_TIME_OUT_MS = 500;

        public enum eStates
        { /* if you change this, you have to change this in master too */
            READY_FOR_NEW_LOT, WAIT_UNTIL_LOT_IN_PROGRESS, READY_TO_MEASURE, MEASUREMENT_RUNNING, MEASUREMENT_EVALUATE,
            COMPENSATION, SHUTDOWN_CMD, DEVICE_ERROR, RS232_ERROR, IO_ERROR, DB_ERROR, EXIT
        };

        public const string MessagePrintLabel = "PRINT_LABEL";
        public const string MessagePrintReport = "PRINT_REPORT";
        public const string DummyTestDataSet = "E-Dummy";
        public const string CompensationTestplan = "Compensation";

        public static readonly string[] CounterVRB_Names = { "VRB_TSide", "VRB_Side", "VRB_Bot", "VRB_Top_4S", "VRB_Reserve1", "VRB_Reserve2" };
        public static readonly string[] CounterTRB_Names = { "TRB_Iso", "TRB_Ind", "TRB_Imp", "TRB_Cap", "TRB_Res1", "TRB_Res2", "TRB_System", "TRB_Reserve" };
    }
}
