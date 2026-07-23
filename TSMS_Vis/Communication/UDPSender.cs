using System;
using System.ComponentModel;
using System.Net;
using System.Net.Sockets;
using System.Windows.Forms;
using TSMS_Vis.Communication;


namespace TSMS_Comm
{
    public class UDPSender
    {
        private UdpClient udpClient = null;
        private IPEndPoint master = null;
        private bool hasdisconnect = false;

        public IPAddress IP
        {
            get { return master.Address; }
        }

        public int Port
        {
            get { return master.Port; }
        }

        public UDPSender(IPEndPoint endPt)
        {
            master = endPt;
            try
            {
                udpClient = new UdpClient(master.Port + 1);       //use a dummy port
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error while creating UDP client!\n" + ex.Message);
            }
        }

        private void bwSender_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if ((!e.Cancelled) && (e.Error == null) && ((bool)e.Result))
                OnCommandSent(new EventArgs());
            else
                OnCommandFailed(new EventArgs());

            ((BackgroundWorker)sender).Dispose();
            GC.Collect();
        }

        private void bwSender_DoWork(object sender, DoWorkEventArgs e)
        {
            e.Result = SendCommandToClient((SendCmd)e.Argument);
        }

        public event CommandSendingFailedEventHandler CommandFailed = null;
        protected virtual void OnCommandFailed(EventArgs e)
        {
            if (CommandFailed != null)
                CommandFailed(this, e);
        }

        public event CommandSentEventHandler CommandSent = null;
        protected virtual void OnCommandSent(EventArgs e)
        {
            if (CommandSent != null)
                CommandSent(this, e);
        }


        #region public methods
        public bool Disconnect()
        {
            if (hasdisconnect)
                return true;

            try
            {
                udpClient.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }

            hasdisconnect = true;
            return true;
        }

        public void SendCommand(SendCmd data)
        {
            BackgroundWorker bwSender = new BackgroundWorker();
            bwSender.DoWork += new DoWorkEventHandler(bwSender_DoWork);
            bwSender.RunWorkerCompleted += new RunWorkerCompletedEventHandler(bwSender_RunWorkerCompleted);
            bwSender.RunWorkerAsync(data);
        }
        #endregion

        #region private methods
        private bool SendBytes(byte[] outStream)
        {
            if ((outStream == null) || (outStream.Length == 0))
                return false;

            try
            {
                //   Byte[] senddata = Encoding.ASCII.GetBytes("Hello World");
                udpClient.Send(outStream, outStream.Length, "localhost", 8501);
                return true;
            }
            catch (Exception)
            {

                return false;
            }
        }

        private bool SendCommandToClient(SendCmd data)
        {
            return SendBytes(data.GetArray());

        }
        #endregion
    }
}


