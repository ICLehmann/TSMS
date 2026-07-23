using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Windows.Navigation;
using TSMS_Vis.Resources;

namespace TSMS_Vis.UserControls
{
    /// <summary>
    /// Interaction logic for UC_Status.xaml
    /// </summary>
    public partial class UC_Status : UserControl
    {
        public UC_Status()
        {
            InitializeComponent();
        }

        bool pat_enabled;
        public bool isPAT_Enabled // PAT function enabled?
        {
            get => pat_enabled;
            set
            {
                pat_enabled = value;
                // PAT button visible or not
                btnRestartPATSampling.Visibility = pat_enabled ? Visibility.Visible : Visibility.Hidden;
            }
        }

        public bool isPAT_ReSampling_Enabled  // sets PAT resampling is enabled or diabled depens on the LOT production status
        {
            set 
            {
                // diable the button when complete pat function is disabled or the sampling has already started
                if (pat_enabled && RestPatSamples == 0 && value != btnRestartPATSampling.IsEnabled) btnRestartPATSampling.IsEnabled = value; 
            }
        }

        public static readonly DependencyProperty MasterProperty = DependencyProperty.Register("Master", typeof(byte), typeof(UC_Status));
        public static readonly DependencyProperty MeasurementsProperty = DependencyProperty.Register("Measurements", typeof(byte), typeof(UC_Status));
        public static readonly DependencyProperty RS232Property = DependencyProperty.Register("RS232", typeof(byte), typeof(UC_Status));
        public static readonly DependencyProperty DatabaseProperty = DependencyProperty.Register("Database", typeof(byte), typeof(UC_Status));
        public static readonly DependencyProperty DigitalIOProperty = DependencyProperty.Register("DigitalIO", typeof(byte), typeof(UC_Status));
        public static readonly DependencyProperty RestPatSamplesProperty = DependencyProperty.Register("RestPatSamples", typeof(ushort), typeof(UC_Status));

        public event RoutedEventHandler InitDevivesClick;
        public event RoutedEventHandler InitRS232Click;
        public event RoutedEventHandler InitDatabaseClick;
        public event RoutedEventHandler InitDigitalIOClick;
        public event RoutedEventHandler RestartPATClick;

        public byte Master
        {
            get { return (byte)GetValue(MasterProperty); }
            set
            {
                SetValue(MasterProperty, value);
                if (value == 0)
                {
                    cbMasterReady.IsChecked = false;
                    cbMasterError.IsChecked = false;
                }
                else if (value == 1)
                {
                    cbMasterReady.IsChecked = true;
                    cbMasterError.IsChecked = false;
                }
                else if (value == 2)
                {
                    cbMasterReady.IsChecked = false;
                    cbMasterError.IsChecked = true;
                }

            }
        }

        public byte Measurements
        {
            get { return (byte)GetValue(MeasurementsProperty); }
            set
            {
                SetValue(MeasurementsProperty, value);
                if (value == 0)
                {
                    cbDevicesReady.IsChecked = false;
                    cbDevicesError.IsChecked = false;
                }
                else if (value == 1)
                {
                    cbDevicesReady.IsChecked = true;
                    cbDevicesError.IsChecked = false;
                }
                else if (value == 2)
                {
                    cbDevicesReady.IsChecked = false;
                    cbDevicesError.IsChecked = true;
                }

                if (value == 2)
                    btnDevices.IsEnabled = true;
                else
                    btnDevices.IsEnabled = false;
            }
        }

        public byte RS232
        {
            get { return (byte)GetValue(RS232Property); }
            set
            {
                SetValue(RS232Property, value);
                if (value == 0)
                {
                    cbRS232Ready.IsChecked = false;
                    cbRS232Error.IsChecked = false;
                }
                else if (value == 1)
                {
                    cbRS232Ready.IsChecked = true;
                    cbRS232Error.IsChecked = false;
                }
                else if (value == 2)
                {
                    cbRS232Ready.IsChecked = false;
                    cbRS232Error.IsChecked = true;
                }

                if (value == 2)
                    btnRS232.IsEnabled = true;
                else
                    btnRS232.IsEnabled = false;
            }
        }

        public byte Database
        {
            get { return (byte)GetValue(DatabaseProperty); }
            set
            {
                SetValue(DatabaseProperty, value);
                if (value == 0)
                {
                    cbDatabaseReady.IsChecked = false;
                    cbDatabaseError.IsChecked = false;
                }
                else if (value == 1)
                {
                    cbDatabaseReady.IsChecked = true;
                    cbDatabaseError.IsChecked = false;
                }
                else if (value == 2)
                {
                    cbDatabaseReady.IsChecked = false;
                    cbDatabaseError.IsChecked = true;
                }

                if (value == 2)
                    btnDatabase.IsEnabled = true;
                else
                    btnDatabase.IsEnabled = false;
            }
        }

        public byte DigitalIO
        {
            get { return (byte)GetValue(DigitalIOProperty); }
            set
            {
                SetValue(DigitalIOProperty, value);
                if (value == 0)
                {
                    cbDigitalIOReady.IsChecked = false;
                    cbDigitalIOError.IsChecked = false;
                }
                else if (value == 1)
                {
                    cbDigitalIOReady.IsChecked = true;
                    cbDigitalIOError.IsChecked = false;
                }
                else if (value == 2)
                {
                    cbDigitalIOReady.IsChecked = false;
                    cbDigitalIOError.IsChecked = true;
                }

                if (value == 2)
                    btnDigitalIO.IsEnabled = true;
                else
                    btnDigitalIO.IsEnabled = false;
            }
        }

        // refresing PAT sample counter
        public ushort RestPatSamples
        {
            get { return (ushort)GetValue(RestPatSamplesProperty); }
            set
            {
                SetValue(RestPatSamplesProperty, value);
                lblRestPatSamples.Content = value.ToString();
            }
        }

        public void Reset()
        {
            cbDevicesReady.IsChecked = false;
            cbDevicesError.IsChecked = false;
            cbRS232Ready.IsChecked = false;
            cbRS232Error.IsChecked = false;
            cbDatabaseReady.IsChecked = false;
            cbDatabaseError.IsChecked = false;
            cbDigitalIOReady.IsChecked = false;
            cbDigitalIOError.IsChecked = false;
        }

        private void btnDevicesClick(object sender, RoutedEventArgs e)
        {
            if (InitDevivesClick != null)
            {
                InitDevivesClick(this, e);
            }
        }

        private void btnDatabaseClick(object sender, RoutedEventArgs e)
        {
            if (InitDatabaseClick != null)
            {
                InitDatabaseClick(this, e);
            }
        }

        private void btnDigitalIOClick(object sender, RoutedEventArgs e)
        {
            if (InitDigitalIOClick != null)
            {
                InitDigitalIOClick(this, e);
            }
        }

        private void btnRS232Click(object sender, RoutedEventArgs e)
        {
            if (InitRS232Click != null)
            {
                InitRS232Click(this, e);
            }
        }

        private void btnRestartNarrowing_Click(object sender, RoutedEventArgs e)
        {
            if (RestartPATClick != null)
            {
                if (Helper.AppHelper.QuestionMsg(Resource.MSG_PAT_RESTART))
                    RestartPATClick(this, e);
            }
        }
    }
}
