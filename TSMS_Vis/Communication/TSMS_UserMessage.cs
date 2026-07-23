using System;
using System.Text;

namespace TSMS_Comm
{
    public struct TSMS_UserMessage
    {
        private ushort len;
        private bool popup;
        private string msg;
        

        public string Message { get => msg; }
        public bool Popup { get => popup; }

        public TSMS_UserMessage(byte[] btArr)
        {
            int n = 1;
            popup = btArr[n] == 1;
            n++;
            len = BitConverter.ToUInt16(btArr, n);
            n += 2;
            msg = Encoding.UTF8.GetString(btArr, n, len);
            msg = msg.TrimEnd('\0');
            n += len;
        }
    }
}
