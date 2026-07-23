using System;
using System.Text;
using TSMS_Vis;

namespace TSMS_Comm
{
    public struct PATConfig
    {
        public byte pat_enabled;
        public double pat_min;
        public double pat_max;
    };

    public struct TSMS_PATUpdate
    {
        private ushort num_configs;

        private PATConfig[] patconfig;
                
        public ushort NumConfigs
        {
            get { return num_configs; }
        }

        public PATConfig[] PatConfig { get => patconfig; }

        public TSMS_PATUpdate(byte[] btArr)
        {
            int n = 1;
            num_configs = BitConverter.ToUInt16(btArr, n);
            n += 2;
            patconfig = new PATConfig[num_configs];
            for (int i = 0; i < patconfig.Length; i++)
            {
                patconfig[i].pat_enabled = btArr[n++];
                patconfig[i].pat_min = BitConverter.ToDouble(btArr, n);
                n += 8;
                patconfig[i].pat_max = BitConverter.ToDouble(btArr, n);
                n += 8;
            }
        }

    }
}
