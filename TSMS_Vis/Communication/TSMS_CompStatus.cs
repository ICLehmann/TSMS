using System;
using System.Text;
using TSMS_Vis;

namespace TSMS_Comm
{
      public struct TSMS_CompStatus
    {
        private ushort num_comps;
        private byte[] comps;
        private string[] label;

        public ushort NumComps
        {
            get { return num_comps; }
        }
               
        public byte[] Comps { get => comps; }
        
        public string[] Label { get => label; }

        public TSMS_CompStatus(byte[] btArr)
        {
            int n = 1;
            num_comps = BitConverter.ToUInt16(btArr, n);
            n += 2;

            comps = new byte[num_comps];
            for (int i = 0; i < comps.Length; i++)
            {
                comps[i] = btArr[n++];
            }

            label = new string[num_comps];
            for (int i = 0; i < comps.Length; i++)
            {
                label[i] = Encoding.UTF8.GetString(btArr, n, TSMS_Consts.TELEGRAM_CHAR_LENGTH);
                label[i] = label[i].TrimEnd('\0');
                n += TSMS_Consts.TELEGRAM_CHAR_LENGTH;
            }
        }
    }
}
