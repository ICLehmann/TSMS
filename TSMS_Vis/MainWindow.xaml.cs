using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Reflection;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using TSMS_Comm;
using TSMS_Vis.Communication;
using TSMS_Vis.Database;
using TSMS_Vis.Resources;

namespace TSMS_Vis
{
    public partial class MainWindow : Window
    {
        private UDPReceiver udp_receiver = null;
        private UDPSender udp_sender = null;
        private DbHandler db = null;

        private TSMS_Setup tsms_setup;
        private TSMS_LotData tsms_lot;

        private CompWindow compWindow = null;
        private MessageDialog msgWindow = null;

        public MainWindow()
        {
            Logger.Write("Program started");
            InitializeComponent();

            tsms_setup = new TSMS_Setup();
            tsms_lot = new TSMS_LotData();

            db = new DbHandler();
        }

        private void SetupUI()
        {
            // title of the main window
            Title = Title + " - Version " + Assembly.GetExecutingAssembly().GetName().Version + " (241120)";

            // creates compensation dialog object
            if (compWindow == null)
            {
                compWindow = new CompWindow();
                compWindow.Owner = this;
            }

            // creates message dialog object
            if (msgWindow == null)
            {
                msgWindow = new MessageDialog();
                msgWindow.Owner = this;
            }

            // setup of the language selector combo
            cbLanguage.Items.Clear();
            cbLanguage.Items.Add(Resource.ENGLISH);
            cbLanguage.Items.Add(Resource.GERMAN);
            cbLanguage.Items.Add(Resource.HUNGARIAN);
            cbLanguage.SelectedIndex = ((App)Application.Current).LangIndex;
            cbLanguage.SelectionChanged += cbLanguage_SelectionChanged;

            // PAT narrowing setup
            var ini_file = new IniFile(TSMS_Consts.INI_FILE);
            string PAT_Mode = ini_file.Read("Mode", "PAT");
            statusPanel.isPAT_Enabled = PAT_Mode == "1";
            statusPanel.isPAT_ReSampling_Enabled = false;   // this function is disabled until the proper machine signal received
         }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            SetupUI();  // setup UI elements first time

            StartUDPReceiver();
            Thread.Sleep(50);

            StartUDPSender();
            Thread.Sleep(50);

            udp_sender.SendCommand(new SendCmd(TSMS_Consts.eTelUItoMaster.TEL_UI_IS_STARTED));
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            udp_receiver.Disconnect();
            udp_sender.Disconnect();
            Thread.Sleep(100);
            Logger.Write("Program closed");
        }

        private void cbLanguage_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (cbLanguage.SelectedIndex == ((App)Application.Current).LangIndex)
                return;

            if (MessageBox.Show(Resource.MSG_SWITCH_LANGUAGE,
                    "Restart program",
                    MessageBoxButton.YesNo,
                    MessageBoxImage.Question) == MessageBoxResult.Yes)
            {
                // Set ini entry and restart program
                string language = "en";
                switch (cbLanguage.SelectedIndex)
                {
                    case 0:
                        language = "en";
                        break;
                    case 1:
                        language = "de";
                        break;
                    case 2:
                        language = "hu";
                        break;
                }

                var ini_file = new IniFile(TSMS_Consts.INI_FILE);
                ini_file.Write("Language", language, "Common");

                System.Diagnostics.Process.Start(Application.ResourceAssembly.Location);
                Application.Current.Shutdown();
            }
            else
                cbLanguage.SelectedIndex = ((App)Application.Current).LangIndex;
        }

        private void StartUDPReceiver()
        {
            IPAddress ipAdress = IPAddress.Parse(TSMS_Consts.IP_ADR);
            IPEndPoint endPt = new IPEndPoint(ipAdress, TSMS_Consts.RECV_PORT);

            udp_receiver = new UDPReceiver(endPt);
            udp_receiver.MachineSignalsReceived += new MachineSignalsReceivedEventHandler(MachineSignalsReceived);
            udp_receiver.StatusReceived += new StatusReceivedEventHandler(StatusReceived);
            udp_receiver.SetupReceived += new SetupReceivedEventHandler(SetupReceived);
            udp_receiver.ValueReceived += new ValueReceivedEventHandler(ValueReceived);
            udp_receiver.LotDataReceived += new LotDataReceivedEventHandler(LotDataReceived);
            udp_receiver.PATUpdateReceived += new PATUpdateReceivedEventHandler(PATUpdateReceived);
            udp_receiver.CounterReceived += new CounterReceivedEventHandler(CounterReceived);
            udp_receiver.ConnectionChanged += new ConnectionChangedEventHandler(UDPConnectionChanged);
            udp_receiver.CompStatusReceived += new CompStatusReceivedEventHandler(CompStatusReceived);
            udp_receiver.UserMessageReceived += new UserMessageReceivedEventHandler(UserMessageReceived);
        }

        private void StartUDPSender()
        {
            IPAddress ipAdress = IPAddress.Parse(TSMS_Consts.IP_ADR);
            IPEndPoint endPt = new IPEndPoint(ipAdress, TSMS_Consts.SEND_PORT);

            udp_sender = new UDPSender(endPt);
        }

        private void MenuItemExit_Click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private void ValueReceived(object sender, TSMS_Value_EventArgs e)
        {
            if (tsms_lot.LotNumber == 0)
                return;

            Dispatcher.BeginInvoke(DispatcherPriority.Normal, (ThreadStart)delegate ()
            {
                if (TrendModule1.Type == e.MvData.Type)
                    TrendModule1.AddValue(e.MvData.Value, e.MvData.PAT_Enabled, e.MvData.PAT_Min ,e.MvData.PAT_Max);

                if (TrendModule2.Type == e.MvData.Type)
                    TrendModule2.AddValue(e.MvData.Value, e.MvData.PAT_Enabled, e.MvData.PAT_Min, e.MvData.PAT_Max);

                if (TrendModule3.Type == e.MvData.Type)
                    TrendModule3.AddValue(e.MvData.Value, e.MvData.PAT_Enabled, e.MvData.PAT_Min, e.MvData.PAT_Max);

                if (TrendModule4.Type == e.MvData.Type)
                    TrendModule4.AddValue(e.MvData.Value, e.MvData.PAT_Enabled, e.MvData.PAT_Min, e.MvData.PAT_Max);

                if (TrendModule5.Type == e.MvData.Type)
                    TrendModule5.AddValue(e.MvData.Value, e.MvData.PAT_Enabled, e.MvData.PAT_Min, e.MvData.PAT_Max);

                if (TrendModule6.Type == e.MvData.Type)
                    TrendModule6.AddValue(e.MvData.Value, e.MvData.PAT_Enabled, e.MvData.PAT_Min, e.MvData.PAT_Max);

                if (TrendModule7.Type == e.MvData.Type)
                    TrendModule7.AddValue(e.MvData.Value, e.MvData.PAT_Enabled, e.MvData.PAT_Min, e.MvData.PAT_Max);

                if (TrendModule8.Type == e.MvData.Type)
                    TrendModule8.AddValue(e.MvData.Value, e.MvData.PAT_Enabled, e.MvData.PAT_Min, e.MvData.PAT_Max);
            });
        }

        private void LotDataReceived(object sender, TSMS_LotData_EventArgs e)
        {
            Dispatcher.BeginInvoke(DispatcherPriority.Normal, (ThreadStart)delegate ()
            {
                tbLotNumber.Text = e.Data.LotNumber.ToString();
                tbTestPlan.Text = e.Data.ProductNumber;
                tbLineId.Text = e.Data.LineID;
                //tbOperatorId.Text = e.Data.OperatorID;

                tsms_lot = e.Data;
                TestConfigPanel.SetConfig(tsms_lot);

                if (e.Data.LotNumber > 0)
                {
                    TrendSetup(TrendModule1);
                    TrendSetup(TrendModule2);
                    TrendSetup(TrendModule3);
                    TrendSetup(TrendModule4);
                    TrendSetup(TrendModule5);
                    TrendSetup(TrendModule6);
                    TrendSetup(TrendModule7);
                    TrendSetup(TrendModule8);
                    btnPrintLabel.IsEnabled = true;
                }
                else
                {
                    ResetControls();
                }

            });
        }

        private void PATUpdateReceived(object sender, TSMS_PATUpdate_EventArgs e)
        {
            if (tsms_lot.LotNumber == 0)
                return;

            Dispatcher.BeginInvoke(DispatcherPriority.Normal, (ThreadStart)delegate ()
            {
                TSMS_PATUpdate tsms_pat;
                tsms_pat = e.Data;
                // updating only the PAT narrowed limits
                if (tsms_pat.NumConfigs == tsms_lot.NumConfigs)
                {
                    for (int i = 0; i < tsms_lot.NumConfigs; i++) {
                        tsms_lot.ValConfig[i].pat_enabled = tsms_pat.PatConfig[i].pat_enabled;
                        tsms_lot.ValConfig[i].pat_min = tsms_pat.PatConfig[i].pat_min;
                        tsms_lot.ValConfig[i].pat_max = tsms_pat.PatConfig[i].pat_max;
                    }
                    TestConfigPanel.SetConfig(tsms_lot);
                }
            });
        }

        private void CounterReceived(object sender, TSMS_Counter_EventArgs e)
        {
            Dispatcher.BeginInvoke(DispatcherPriority.Normal, (ThreadStart)delegate ()
            {
                CounterPanel.SetCounter(e.Data);
            });
        }

        private void SetupReceived(object sender, TSMS_Setup_EventArgs e)
        {
            Dispatcher.BeginInvoke(DispatcherPriority.Normal, (ThreadStart)delegate ()
            {
                lMachine.Content = e.Data.Machine;
                tsms_setup = e.Data;
                CounterPanel.Setup(e.Data.Measurements);
                TestConfigPanel.Setup(e.Data.Measurements);
                TrendModule1.SetComboBox(e.Data.Measurements, 0);
                TrendModule2.SetComboBox(e.Data.Measurements, 1);
                TrendModule3.SetComboBox(e.Data.Measurements, 2);
                TrendModule4.SetComboBox(e.Data.Measurements, 3);
                TrendModule5.SetComboBox(e.Data.Measurements, 4);
                TrendModule6.SetComboBox(e.Data.Measurements, 5);
                TrendModule7.SetComboBox(e.Data.Measurements, 8);
                TrendModule8.SetComboBox(e.Data.Measurements, 9);
                Histogram1.SetComboBox(e.Data.Measurements, 0);
                Histogram2.SetComboBox(e.Data.Measurements, 1);
                Histogram3.SetComboBox(e.Data.Measurements, 2);
                Histogram4.SetComboBox(e.Data.Measurements, 3);
                Histogram5.SetComboBox(e.Data.Measurements, 4);
                Histogram6.SetComboBox(e.Data.Measurements, 5);
                Histogram7.SetComboBox(e.Data.Measurements, 8);
                Histogram8.SetComboBox(e.Data.Measurements, 9);
                Probability1.SetComboBox(e.Data.Measurements, 8);
                Probability2.SetComboBox(e.Data.Measurements, 9);
                Probability3.SetComboBox(e.Data.Measurements, 2);
                Probability4.SetComboBox(e.Data.Measurements, 3);
            });
        }

        private void MachineSignalsReceived(object sender, TSMS_MachineSignals_EventArgs e)
        {
            Dispatcher.BeginInvoke(DispatcherPriority.Normal, (ThreadStart)delegate ()
            {
                ioPanel.SetSignals(e.Data);
                // PAt resampling is enabled when the machine in LOT in progress mode
                statusPanel.isPAT_ReSampling_Enabled = e.Data.LotInProgress > 0;
            });
        }

        private void StatusReceived(object sender, TSMS_Status_EventArgs e)
        {
            Dispatcher.BeginInvoke(DispatcherPriority.Normal, (ThreadStart)delegate ()
            {
                TSMS_Consts.eStates state = (TSMS_Consts.eStates)e.Data.StateMachine;
                lStatus.Foreground = System.Windows.Media.Brushes.Green;
                switch (state)
                {
                    case TSMS_Consts.eStates.READY_FOR_NEW_LOT:
                        lStatus.Content = Resource.STATE_READY_FOR_NEW_LOT;
                        break;
                    case TSMS_Consts.eStates.WAIT_UNTIL_LOT_IN_PROGRESS:
                        lStatus.Content = Resource.STATE_WAIT_UNTIL_LOT_IN_PROGRESS;
                        break;
                    case TSMS_Consts.eStates.READY_TO_MEASURE:
                        lStatus.Content = Resource.STATE_READY_TO_MEASURE;
                        break;
                    case TSMS_Consts.eStates.MEASUREMENT_RUNNING:
                        lStatus.Content = Resource.STATE_MEASUREMENT_RUNNING;
                        break;
                    case TSMS_Consts.eStates.MEASUREMENT_EVALUATE:
                        lStatus.Content = Resource.STATE_MEASUREMENT_EVALUATE;
                        break;
                    case TSMS_Consts.eStates.COMPENSATION:
                        lStatus.Content = Resource.STATE_COMPENSATION;
                        compWindow?.Show();
                        break;
                    case TSMS_Consts.eStates.SHUTDOWN_CMD:
                        lStatus.Foreground = System.Windows.Media.Brushes.Red;
                        lStatus.Content = Resource.STATE_SHUTDOWN;
                        break;
                    case TSMS_Consts.eStates.DEVICE_ERROR:
                        lStatus.Foreground = System.Windows.Media.Brushes.Red;
                        lStatus.Content = Resource.STATE_DEVICE_ERROR;
                        break;
                    case TSMS_Consts.eStates.RS232_ERROR:
                        lStatus.Foreground = System.Windows.Media.Brushes.Red;
                        lStatus.Content = Resource.STATE_RS232_ERROR;
                        break;
                    case TSMS_Consts.eStates.IO_ERROR:
                        lStatus.Foreground = System.Windows.Media.Brushes.Red;
                        lStatus.Content = Resource.STATE_IO_ERROR;
                        break;
                    case TSMS_Consts.eStates.DB_ERROR:
                        lStatus.Foreground = System.Windows.Media.Brushes.Red;
                        lStatus.Content = Resource.STATE_DB_ERROR;
                        break;
                }

                statusPanel.Measurements = e.Data.Measurements;
                statusPanel.RS232 = e.Data.RS232;
                statusPanel.Database = e.Data.Database;
                statusPanel.DigitalIO = e.Data.DigitalIO;
                statusPanel.RestPatSamples = e.Data.RestPatSamples;

                lRestDummy.Content = e.Data.RestDummyHours.ToString();
                lRestComp.Content = e.Data.RestCompHours.ToString();

                if (state != TSMS_Consts.eStates.COMPENSATION && compWindow != null && compWindow.ShowActivated)
                {
                    compWindow.Close();
                }
            });
        }

        private void CompStatusReceived(object sender, TSMS_CompStatus_EventArgs e)
        {
            Dispatcher.BeginInvoke(DispatcherPriority.Normal, (ThreadStart)delegate ()
            {
                compWindow?.UpdateStatus(e.Data);
            });
        }

        private void UserMessageReceived(object sender, TSMS_UserMessage_EventArgs e)
        {
            Dispatcher.BeginInvoke(DispatcherPriority.Normal, (ThreadStart)delegate ()
            {
                if (e.Data.Popup)
                {
                    msgWindow.SetMessage(e.Data.Message);
                    if (!msgWindow.IsVisible)
                        msgWindow.Show();
                }
                else
                {
                    // master send this message when the LOT is finished
                    if (e.Data.Message == TSMS_Consts.MessagePrintReport)
                    {
                        var lot_num = tsms_lot.LotNumber;
                        var product = tsms_lot.ProductNumber;

                        // PDF export 
                        var ini_file = new IniFile(TSMS_Consts.INI_FILE);
                        string path = ini_file.Read("ReportPath", "Common");
                        // creates directory if not exist
                        Helper.TSMSFileSystem.CheckDirectory(ref path, @"C:\Users\Public\Documents");

                        Report.Report report = new Report.Report(lot_num, new Report.ReportType());
                        report.WriteToPdf(path);
                        Logger.Write(string.Format("Report '{0}' as PDF saved", lot_num));

                        // send to printer
                        string print_to_default = ini_file.Read("ReportToDefaultPrinter", "Common");
                        if (print_to_default.ToLower() == "true")
                        {
                            report.SendToDefaultPrinter();
                            Logger.Write(string.Format("Report '{0}' send to printer", lot_num));
                        }

                        // Wodi export 
                        {
                            path = ini_file.Read("WodiExportPath", "Common");
                            // creates directory if not exist
                            Helper.TSMSFileSystem.CheckDirectory(ref path, @"C:\Users\Public\Documents");

                            WodiExport wodiExport = new WodiExport();
                            wodiExport.writeFile(lot_num, path);
                        }

                        // export csv                        
                        {
                            path = ini_file.Read("CsvExportPath", "Common");
                            // creates directory if not exist
                            Helper.TSMSFileSystem.CheckDirectory(ref path, @"C:\Users\Public\Documents");

                            db.ExportMeasurmentsToCSV(lot_num, path);

                            // ---- alternatively --> csv export with external batch file ----
                            //var process = new Process();
                            //process.StartInfo.FileName = @"CsvExport.bat";
                            //process.StartInfo.Arguments = string.Format("{0}", tsms_lot.LotNumber);
                            //bool b = process.Start();
                        }

                        // print to the label printer
                        PrintLabel(lot_num, product);
                    }
                    else if (e.Data.Message == TSMS_Consts.MessagePrintLabel)
                    {
                        PrintLabel(tsms_lot.LotNumber, tsms_lot.ProductNumber);
                    }
                    else
                        lErrorMsg.Content = e.Data.Message;
                }
            });
        }

        private void ResetControls()
        {
            Dispatcher.BeginInvoke(DispatcherPriority.Normal, (ThreadStart)delegate ()
            {
                statusPanel.Reset();
                ioPanel.Reset();
                CounterPanel.Reset();

                TrendModule1.Reset();
                TrendModule2.Reset();
                TrendModule3.Reset();
                TrendModule4.Reset();
                TrendModule5.Reset();
                TrendModule6.Reset();
                TrendModule7.Reset();
                TrendModule8.Reset();

                btnPrintLabel.IsEnabled = false;

                tbLotNumber.Text = "";
                tbTestPlan.Text = "";
                tbLineId.Text = "";

                lRestComp.Content = "0";
                lRestDummy.Content = "0";

                compWindow?.Close();

                lErrorMsg.Content = "";
                lStatus.Content = "";
            });
        }

        private void UDPConnectionChanged(object sender, ConnectionChanged_EventArgs e)
        {
            Dispatcher.BeginInvoke(DispatcherPriority.Normal, (ThreadStart)delegate ()
            {
                if (e.MasterReady)
                {
                    statusPanel.Master = 1;
                }
                else
                {
                    statusPanel.Master = 2;
                    lStatus.Content = "No Answer from Master";
                    lStatus.Foreground = System.Windows.Media.Brushes.Red;
                    lErrorMsg.Content = "";
                    ResetControls();
                }
            });
        }

        //Send command to the Master application to reinitialize RS232
        private void statusPanel_InitRS232Click(object sender, RoutedEventArgs e)
        {
            udp_sender.SendCommand(new SendCmd(TSMS_Consts.eTelUItoMaster.TEL_UI_INIT_RS232));
        }

        //Send command to the Master application to reinitialize database
        private void statusPanel_InitDatabaseClick(object sender, RoutedEventArgs e)
        {
            udp_sender.SendCommand(new SendCmd(TSMS_Consts.eTelUItoMaster.TEL_UI_INIT_DB));
        }

        //Send command to the Master application to reinitialize devices
        private void statusPanel_InitDevicesClick(object sender, RoutedEventArgs e)
        {
            udp_sender.SendCommand(new SendCmd(TSMS_Consts.eTelUItoMaster.TEL_UI_INIT_DEVICES));
        }

        //Send command to the Master application to reinitialize digital IO
        private void statusPanel_InitDigitalIOClick(object sender, RoutedEventArgs e)
        {
            udp_sender.SendCommand(new SendCmd(TSMS_Consts.eTelUItoMaster.TEL_UI_INIT_IO));
        }

        //Send command to the Master application to restart PAT sampling
        private void statusPanel_RestartPATClick(object sender, RoutedEventArgs e)
        {
            udp_sender.SendCommand(new SendCmd(TSMS_Consts.eTelUItoMaster.TEL_UI_RESTART_PAT));
        }

        private void btnResetZoom_Click(object sender, RoutedEventArgs e)
        {
            TrendModule1.ZoomReset();
            TrendModule2.ZoomReset();
            TrendModule3.ZoomReset();
            TrendModule4.ZoomReset();
            TrendModule5.ZoomReset();
            TrendModule6.ZoomReset();
            TrendModule7.ZoomReset();
            TrendModule8.ZoomReset();
        }

        private void TrendSetup(UserControls.UC_TrendModule trend)
        {
            if (tsms_lot.ValConfig != null && tsms_lot.ValConfig.Length > trend.Type)
            {
                trend.SetupChart(tsms_lot.ValConfig[trend.Type].min,
                    tsms_lot.ValConfig[trend.Type].max,
                    tsms_lot.ValConfig[trend.Type].nom,
                    tsms_lot.ValConfig[trend.Type].prefix + tsms_setup.Measurements[trend.Type].Unit);
            }
        }

        private void TrendModule_TypeChanged(object sender, System.EventArgs e)
        {
            TrendSetup((UserControls.UC_TrendModule)sender);
        }

        private void UpdateHistogram(UserControls.UC_HistogramModule hist)
        {
            if (tsms_lot.LotNumber == 0)
                return;

            uint type = (uint)hist.Type;
            if (type < tsms_setup.Measurements.Length)
            {
                List<double> values;
                double min;
                double max;
                ClassLibrary.BasicTypes.HistogramData dataH;
                
                // shows PAT narrowed histogram if the related menu item is checked
                if (mnuShowPATHisto.IsChecked && tsms_lot.ValConfig[type].pat_enabled != 0)
                {
                    // PAT narrowing
                    int startpart = db.GetLastNarrowingStart(tsms_lot.LotNumber);
                    values = db.GetMeasurementData(tsms_lot.LotNumber, tsms_setup.Measurements[type].Name, startpart, null, "!= 'error'");

                    min = tsms_lot.ValConfig[type].pat_min;
                    max = tsms_lot.ValConfig[type].pat_max;

                    dataH = ClassLibrary.ChartMethod.CreateHistogramData(min, max, min, max, values, true, startpart);
                }
                else
                {
                    // no narrowing
                    values = db.GetMeasurementData(tsms_lot.LotNumber, tsms_setup.Measurements[type].Name, null, null, "!= 'error'");

                    min = tsms_lot.ValConfig[type].min;
                    max = tsms_lot.ValConfig[type].max;

                    dataH = ClassLibrary.ChartMethod.CreateHistogramData(min, max, min, max, values, false);
                }
                
                string title = tsms_setup.Measurements[type].Name + " in " + tsms_setup.Measurements[type].Unit;
                hist.DrawHistogram(title, dataH);
            }
        }

        private void UpdateProbability(UserControls.UC_ProbabilityModule prob)
        {
            if (tsms_lot.LotNumber == 0)
                return;

            uint type = (uint)prob.Type;
            if (type < tsms_setup.Measurements.Length)
            {
                var values = db.GetMeasurementData(tsms_lot.LotNumber, tsms_setup.Measurements[type].Name, null, null, "= 'pass'");
                double min = tsms_lot.ValConfig[type].min;
                double max = tsms_lot.ValConfig[type].max;
                double min_n = min;     // narrow is the same in  moment
                double max_n = max;
                var dataP = ClassLibrary.ChartMethod.CreateProbabilityPoints(min_n, max_n, min, max, values);
                prob.DrawProbabilityPlot(dataP);
            }
        }

        private void RefreshCharts()
        {
            if (TabHist1 != null && TabHist1.IsSelected)
            {
                UpdateHistogram(Histogram1);
                UpdateHistogram(Histogram2);
                UpdateHistogram(Histogram3);
                UpdateHistogram(Histogram4);
            }

            if (TabHist2 != null && TabHist2.IsSelected)
            {
                UpdateHistogram(Histogram5);
                UpdateHistogram(Histogram6);
                UpdateHistogram(Histogram7);
                UpdateHistogram(Histogram8);
            }

            if (TabProb != null && TabProb.IsSelected)
            {
                UpdateProbability(Probability1);
                UpdateProbability(Probability2);
                UpdateProbability(Probability3);
                UpdateProbability(Probability4);
            }
        }

        private void TabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            RefreshCharts();
        }

        private void btnRefresh(object sender, RoutedEventArgs e)
        {
            RefreshCharts();
        }

        private void PrintLabel(ulong lot_num, string product)
        {           
            var LabelCounter = db.GetLabelCounter(lot_num);
            var printer = LabelPrinter.GetPrinter();
            if (printer.PrintLabel(lot_num, product, LabelCounter))
            {
                LabelCounter++;
                db.UpdateLabelCounter(lot_num, LabelCounter);
                Logger.Write(string.Format("label {0}/{1}/{2} printed", lot_num, tsms_lot.ProductNumber, LabelCounter));
            }
        }

        private void PrintLabelClick(object sender, RoutedEventArgs e)
        {
            PrintLabel(tsms_lot.LotNumber, tsms_lot.ProductNumber);
        }

        private void PrintLabelAdvanced(object sender, RoutedEventArgs e)
        {
            var dialog = new RePrintLabelDialog(ref db);
            dialog.Owner = this;
            dialog.ShowDialog();
        }

        private void btnPrintDocument_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new PrintReportDialog(ref db);
            dialog.Owner = this;
            dialog.ShowDialog();
        }
    }
}
