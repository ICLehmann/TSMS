using System;

namespace TSMS_Comm
{
    public struct TSMS_Status
    {
        public byte StateMachine { get; }
        public byte Measurements { get; }
        public byte RS232 { get; }
        public byte Database { get; }
        public byte DigitalIO { get; }
        public ushort RestDummyHours { get; } 
        public ushort RestCompHours { get; }
        public ushort RestPatSamples { get; }

        public TSMS_Status(byte[] btArr)
        {
            int n = 1;
            StateMachine = btArr[n++];
            Measurements = btArr[n++];
            RS232 = btArr[n++];
            Database = btArr[n++];
            DigitalIO = btArr[n++];
            RestDummyHours = BitConverter.ToUInt16(btArr, n);
            n += 2;
            RestCompHours = BitConverter.ToUInt16(btArr, n);
            n += 2;
            RestPatSamples = BitConverter.ToUInt16(btArr, n);
            n += 2;
        }
    }
}
