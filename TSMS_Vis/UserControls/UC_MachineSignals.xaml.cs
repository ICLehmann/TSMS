using System.Windows;
using System.Windows.Controls;
using TSMS_Comm;

namespace TSMS_Vis.UserControls
{
    /// <summary>
    /// Interaction logic for UC_MachineSignals.xaml
    /// </summary>
    public partial class UC_MachineSignals : UserControl
    {
        public UC_MachineSignals()
        {
            InitializeComponent();
        }
        

        public void SetSignals(TSMS_MachineSignals signals)
        {
            cbMachineReady.IsChecked = signals.MachineReady > 0;
            cbMachineEmpty.IsChecked = signals.MachineEmpty > 0;
            cbLotInProgress.IsChecked = signals.LotInProgress > 0;
            cbLotStart.IsChecked = signals.LotStart > 0;
            cbLotEnd.IsChecked = signals.LotEnd > 0;
            cbLotPause.IsChecked = signals.LotPause > 0;
            cbLotDiscard.IsChecked = signals.LotDiscard > 0;
            cbCompensation.IsChecked = signals.Compensation > 0;
            cbEDummy.IsChecked = signals.EDummy > 0;
            cbInitialization.IsChecked = signals.Initialization > 0;
        }

        public void Reset()
        {
            cbMachineReady.IsChecked = false;
            cbMachineEmpty.IsChecked = false;
            cbLotInProgress.IsChecked = false;
            cbLotStart.IsChecked = false;
            cbLotEnd.IsChecked = false;
            cbLotPause.IsChecked = false;
            cbLotDiscard.IsChecked = false;
            cbCompensation.IsChecked = false;
            cbEDummy.IsChecked = false;
            cbInitialization.IsChecked = false;
        }
    }
}
