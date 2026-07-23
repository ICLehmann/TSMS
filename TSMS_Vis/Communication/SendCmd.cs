namespace TSMS_Vis.Communication
{
    public struct SendCmd
    {
        public const byte id = (byte)'X';
        public byte Cmd;

        public SendCmd(TSMS_Consts.eTelUItoMaster cmd)
        {
            Cmd = (byte)cmd;
        }

        public byte[] GetArray()
        {
            //byte[] cmdBytes = BitConverter.GetBytes(Cmd); // for other types
            //byte[] retVal = new byte[id.Length + cmdBytes.Length];
            byte[] retVal = new byte[2];
            retVal[0] = id;
            retVal[1] = Cmd;
            //Buffer.BlockCopy(idBytes,0, retVal, 0, idBytes.Length);
            //Buffer.BlockCopy(cmdBytes, 0, retVal, idBytes.Length, cmdBytes.Length);
            return retVal;
        }
    }
}
