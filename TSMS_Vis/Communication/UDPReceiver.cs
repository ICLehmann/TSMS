using System;
using System.ComponentModel;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Windows.Forms;
using TSMS_Vis;

namespace TSMS_Comm
{
    public class UDPReceiver
    {
        private BackgroundWorker worker = null;
        private UdpClient udpClient = null;
        private IPEndPoint master = null;
        private bool hasdisconnect = false;
        private bool _master_ready = false;

        private void SetMasterReady(bool value)
        {
            if (_master_ready != value)
            {
                _master_ready = value;
                OnConnectionChanged(new ConnectionChanged_EventArgs(_master_ready));
            }
        }

        public UDPReceiver(IPEndPoint endPt)
        {
            master = endPt;
            try
            {
                udpClient = new UdpClient(master.Port);
            }
            catch (Exception ex)
            {
                string msg = "Error while creating UDP client!";
                Logger.Write(msg);
                MessageBox.Show(msg + "\n" + ex.Message);
            }

            worker = new BackgroundWorker();
            worker.WorkerSupportsCancellation = true;
            worker.DoWork += new DoWorkEventHandler(StartReceive);
            worker.RunWorkerAsync();
        }

        public event ValueReceivedEventHandler ValueReceived = null;
        protected virtual void OnValueReceived(TSMS_Value_EventArgs e)
        {
            if (ValueReceived != null)
                ValueReceived(this, e);
        }

        public event LotDataReceivedEventHandler LotDataReceived = null;
        protected virtual void OnLotDataReceived(TSMS_LotData_EventArgs e)
        {
            if (LotDataReceived != null)
                LotDataReceived(this, e);
        }

        public event PATUpdateReceivedEventHandler PATUpdateReceived = null;
        protected virtual void OnPATUpdateReceived(TSMS_PATUpdate_EventArgs e)
        {
            if (PATUpdateReceived != null)
                PATUpdateReceived(this, e);
        }

        public event CounterReceivedEventHandler CounterReceived = null;
        protected virtual void OnCounterReceived(TSMS_Counter_EventArgs e)
        {
            if (CounterReceived != null)
                CounterReceived(this, e);
        }

        public event SetupReceivedEventHandler SetupReceived = null;
        protected virtual void OnSetupReceived(TSMS_Setup_EventArgs e)
        {
            if (SetupReceived != null)
                SetupReceived(this, e);
        }

        public event MachineSignalsReceivedEventHandler MachineSignalsReceived = null;
        protected virtual void OnMachineSignalsReceived(TSMS_MachineSignals_EventArgs e)
        {
            if (MachineSignalsReceived != null)
                MachineSignalsReceived(this, e);
        }

        public event StatusReceivedEventHandler StatusReceived = null;
        protected virtual void OnStatusReceived(TSMS_Status_EventArgs e)
        {
            if (StatusReceived != null)
                StatusReceived(this, e);
        }

        public event CompStatusReceivedEventHandler CompStatusReceived = null;
        protected virtual void OnCompStatusReceived(TSMS_CompStatus_EventArgs e)
        {
            if (CompStatusReceived != null)
                CompStatusReceived(this, e);
        }

        public event UserMessageReceivedEventHandler UserMessageReceived = null;
        protected virtual void OnUserMessageReceived(TSMS_UserMessage_EventArgs e)
        {
            if (UserMessageReceived != null)
                UserMessageReceived(this, e);
        }

        public event ConnectionChangedEventHandler ConnectionChanged = null;
        protected virtual void OnConnectionChanged(ConnectionChanged_EventArgs e)
        {
            if (ConnectionChanged != null)
                ConnectionChanged(this, e);
        }

        private void StartReceive(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker bg_worker = sender as BackgroundWorker;

            try
            {
                udpClient.Client.ReceiveTimeout = TSMS_Consts.RECEIVE_TIME_OUT_MS;
                SetMasterReady(false);
                IPEndPoint endPoint = new IPEndPoint(master.Address, master.Port);
                while (!bg_worker.CancellationPending)
                {
                    try
                    {
                        byte[] byte_array = udpClient.Receive(ref endPoint);

                        if (byte_array.Length > 0)
                        {
                            if (byte_array[0] == (byte)TSMS_Consts.eTelMasterToUI.TEL_MEAS_VALUE)
                            {
                                TSMS_MeasuredValue value = new TSMS_MeasuredValue(byte_array);
                                OnValueReceived(new TSMS_Value_EventArgs(value));
                            }
                            else if (byte_array[0] == (byte)TSMS_Consts.eTelMasterToUI.TEL_LOT)
                            {
                                TSMS_LotData lot_header = new TSMS_LotData(byte_array);
                                OnLotDataReceived(new TSMS_LotData_EventArgs(lot_header));
                            }
                            else if (byte_array[0] == (byte)TSMS_Consts.eTelMasterToUI.TEL_COUNTER)
                            {
                                TSMS_Counter counter = new TSMS_Counter(byte_array);
                                OnCounterReceived(new TSMS_Counter_EventArgs(counter));
                            }
                            else if (byte_array[0] == (byte)TSMS_Consts.eTelMasterToUI.TEL_SETUP)
                            {
                                TSMS_Setup setup = new TSMS_Setup(byte_array);
                                OnSetupReceived(new TSMS_Setup_EventArgs(setup));
                            }
                            else if (byte_array[0] == (byte)TSMS_Consts.eTelMasterToUI.TEL_MACHINE)
                            {
                                TSMS_MachineSignals signals = new TSMS_MachineSignals(byte_array);
                                OnMachineSignalsReceived(new TSMS_MachineSignals_EventArgs(signals));
                            }
                            else if (byte_array[0] == (byte)TSMS_Consts.eTelMasterToUI.TEL_STATUS)
                            {
                                TSMS_Status status = new TSMS_Status(byte_array);
                                OnStatusReceived(new TSMS_Status_EventArgs(status));
                            }
                            else if (byte_array[0] == (byte)TSMS_Consts.eTelMasterToUI.TEL_COMP_STATUS)
                            {
                                TSMS_CompStatus status = new TSMS_CompStatus(byte_array);
                                OnCompStatusReceived(new TSMS_CompStatus_EventArgs(status));
                            }
                            else if (byte_array[0] == (byte)TSMS_Consts.eTelMasterToUI.TEL_USER_MSG)
                            {
                                TSMS_UserMessage msg = new TSMS_UserMessage(byte_array);
                                OnUserMessageReceived(new TSMS_UserMessage_EventArgs(msg));
                            }
                            else if (byte_array[0] == (byte)TSMS_Consts.eTelMasterToUI.TEL_PAT_UPDATE)
                            {
                                TSMS_PATUpdate pat_update = new TSMS_PATUpdate(byte_array);
                                OnPATUpdateReceived(new TSMS_PATUpdate_EventArgs(pat_update));
                            }
                            else
                            {
                                string msg = "Wrong UDP Telegram!";
                                Logger.Write(msg);
                                System.Windows.MessageBox.Show(msg);
                            }

                            SetMasterReady(true);
                        }

                        Thread.Sleep(10);
                        Application.DoEvents();
                    }
                    catch (Exception ex)
                    {
                        SocketError socketError = SocketError.Success;
                        if (ex is SocketException)
                            socketError = ((SocketException)ex).SocketErrorCode;

                        if (socketError == SocketError.TimedOut)
                        {
                            SetMasterReady(false);
                            continue;
                        }
                        else 
                            throw;  
                    }

                }
            }
            catch (Exception)
            {

            }

            Disconnect();
            e.Cancel = true;
        }

        public bool Disconnect()
        {
            if (hasdisconnect)
                return true;

            if (!worker.CancellationPending)
                worker.CancelAsync();

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

    }
}


