using System;
using TSMS_Vis;

namespace TSMS_Comm
{
    public struct TSMS_Counter
    {
        private uint cnt_total;
        private uint cnt_good;
        private ushort num_counter;
        private uint[] res_pass;
        private uint[] res_low;
        private uint[] res_high;
        private uint[] res_fail;

        public uint CntTotal
        {
            get { return cnt_total; }
        }
        public uint CntGood
        {
            get { return cnt_good; }
        }

        public uint[] ResultPass { get => res_pass; }
        public uint[] ResultLow { get => res_low; }
        public uint[] ResultHigh { get => res_high; }
        public uint[] ResultFail { get => res_fail; }

        public TSMS_Counter(byte[] btArr)
        {
            int n = 1;
            cnt_total = BitConverter.ToUInt32(btArr, n);
            n += 4;
            cnt_good = BitConverter.ToUInt32(btArr, n);
            n += 4;
            num_counter = BitConverter.ToUInt16(btArr, n);
            n += 2;
            res_pass = new uint[num_counter];
            for (int i = 0; i < res_pass.Length; i++)
            {
                res_pass[i] = BitConverter.ToUInt32(btArr, n);
                n += 4;
            }
            res_low = new uint[num_counter];
            for (int i = 0; i < res_low.Length; i++)
            {
                res_low[i] = BitConverter.ToUInt32(btArr, n);
                n += 4;
            }

            res_high = new uint[num_counter];
            for (int i = 0; i < res_high.Length; i++)
            {
                res_high[i] = BitConverter.ToUInt32(btArr, n);
                n += 4;
            }

            res_fail = new uint[num_counter];
            for (int i = 0; i < res_fail.Length; i++)
            {
                res_fail[i] = BitConverter.ToUInt32(btArr, n);
                n += 4;
            }
        }

    }
}
