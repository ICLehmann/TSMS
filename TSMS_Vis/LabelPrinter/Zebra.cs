using System;
using System.IO;
using System.Net.Sockets;
using System.Runtime.CompilerServices;
using System.Text;
using System.Windows;

namespace TSMS_Vis
{
    //  Tested with Zebra ZD421
    public class Zebra : LabelPrinterInterface
    {
        private bool Connect(ref TcpClient tcp, string ipAddress, int port)
        {
            if (!tcp.ConnectAsync(ipAddress, port).Wait(1000))
                return false;
            byte[] buffer = Encoding.ASCII.GetBytes("~HS");
            tcp.GetStream().Write(buffer, 0, buffer.Length);

            byte[] status = new byte[256];
            if (tcp.GetStream().Read(status, 0, status.Length) > 0 && status[0] == 2 && status[33] == 3)    // checks STX and ETX bytes
            {
                Logger.Write("Zebra printer initialized");
            }
            else
            {
                return false;
            }

            return true;
        }

        private string GetTime(string placeHolder)
        {
            DateTime now = DateTime.Now;
            string timeStr = placeHolder.Substring("<Timestamp".Length, placeHolder.Length - "<Timestamp".Length - 1);
            timeStr = timeStr.Replace("%d", now.Day.ToString("00"));
            timeStr = timeStr.Replace("%m", now.Month.ToString("00"));
            timeStr = timeStr.Replace("%Y", now.Year.ToString());
            timeStr = timeStr.Replace("%H", now.Hour.ToString("00"));
            timeStr = timeStr.Replace("%M", now.Minute.ToString("00"));
            return timeStr;
        }

        private string ReplaceTimeString(string line)
        {
            int from = line.IndexOf("<");
            if (from == -1)
                return line;
            int to = line.IndexOf(">");
            if (to == -1)
                return line;
            int placeHolderLen = to - from + 1;
            string timePlaceHolder = line.Substring(from, placeHolderLen);
            return line.Substring(0, from) + GetTime(timePlaceHolder) + line.Substring(to + 1);
        }

        public bool PrintLabel(ulong LotNum, string ProdNum, int LabelCounter)
        {
            TcpClient tcp = new TcpClient();
            if (!Connect(ref tcp, TSMS_Consts.IP_ADR_PRINTER, TSMS_Consts.PORT_PRINTER))
            {
                string msg = "Unable to connect label printer";
                Logger.Write(msg);
                MessageBox.Show(msg, "Printer error");
                return false;
            }

            string printerString;
            string templateFile = TSMS_Consts.LABEL_TEMPLATE_FILE;
            try
            {
                using (StreamReader tmpFile = new StreamReader(templateFile))
                {
                    StringBuilder textStream = new StringBuilder();
                    string line;
                    while ((line = tmpFile.ReadLine()) != null)
                    {
                        if (line.Length == 0 || line[0] == ';')
                            continue;
                        if (line.Contains("<Timestamp"))
                            textStream.AppendLine(ReplaceTimeString(line));
                        else
                            textStream.AppendLine(line);
                    }
                    printerString = textStream.ToString();
                    string str = (LabelCounter + 1).ToString("00");
                    printerString = printerString.Replace("<Counter>", str);
                    printerString = printerString.Replace("<LotNo>", LotNum.ToString());
                    printerString = printerString.Replace("<PartNo>", ProdNum);

                }
            }
            catch (Exception)
            {
                string msg = "Unable to open template file '" + templateFile + "'.";
                Logger.Write(msg);
                MessageBox.Show(msg, "Printer error");
                tcp.Close();
                return false;
            }

            Logger.Write("Send message to label printer: " + printerString);

            byte[] sendBytes = Encoding.ASCII.GetBytes(printerString);
            tcp.GetStream().Write(sendBytes, 0, sendBytes.Length);
            tcp.Close();
            return true;
        }
    }
}