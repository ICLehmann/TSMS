using System;

namespace TSMS_Comm
{

    public struct TSMS_MeasuredValue
    {
        private ushort type;
        private double value;
        private byte pat_enabled;
        private double pat_min;
        private double pat_max;

        public ushort Type
        {
            get { return type; }
        }
        public double Value
        {
            get { return value; }
        }

        public byte PAT_Enabled
        {
            get { return pat_enabled; }
        }

        public double PAT_Min
        {
            get { return pat_min; }
        }

        public double PAT_Max
        {
            get { return pat_max; }
        }

        public TSMS_MeasuredValue(ushort Type, double Value, double Min, double Nom, double Max, byte PAT_Enabled, double PAT_Min, double PAT_Max)
        {
            this.type = Type;  this.value = Value; this.pat_enabled = PAT_Enabled; this.pat_min = PAT_Min; this.pat_max = PAT_Max;
        }

        public TSMS_MeasuredValue(byte[] btArr)
        {
            int n = 1;
            type = BitConverter.ToUInt16(btArr, n);
            n += 2;
            value = BitConverter.ToDouble(btArr, n);
            n += 8;
            pat_enabled = btArr[n++];
            if (pat_enabled != 0)
            {
                pat_min = BitConverter.ToDouble(btArr, n);
                n += 8;
                pat_max = BitConverter.ToDouble(btArr, n);
            }
            else
            {
                pat_min = 0; pat_max = 0;
            }
        }

    }
}
