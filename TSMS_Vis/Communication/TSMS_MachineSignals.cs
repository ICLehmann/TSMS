namespace TSMS_Comm
{
    public struct TSMS_MachineSignals
    {
        public byte MachineReady { get; }
        public byte MachineEmpty { get; }
        public byte LotInProgress { get; }
        public byte LotStart{ get; }
        public byte LotEnd { get; }
        public byte LotPause { get; }
        public byte LotDiscard { get; }
        public byte Compensation { get; }
        public byte EDummy { get; }
        public byte Initialization { get; }

        public TSMS_MachineSignals(byte[] btArr)
        {
            int n = 1;
            MachineReady = btArr[n++];
            MachineEmpty = btArr[n++];
            LotInProgress = btArr[n++];
            LotStart = btArr[n++]; 
            LotEnd = btArr[n++];
            LotPause = btArr[n++];
            LotDiscard = btArr[n++];
            Compensation = btArr[n++];
            EDummy = btArr[n++];
            Initialization = btArr[n++];
        }
    }
}
