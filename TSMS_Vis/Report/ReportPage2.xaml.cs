using System.Windows;

namespace TSMS_Vis.Report
{
    public partial class ReportPage2 : Window
    {
        public ReportPage2(ReportData data, int page, int num_pages)
        {
            InitializeComponent();
            Header.SetPage(page, num_pages);
            Header.SetLotData(data);
            Header.SetWarning(false);

            if (data.reportType.CompleteLOT)
            {
                ErrorDetail.SetDataCompleteLOT(data);
                MachineCounters.SetData(data);
                MachineCounters.Visibility = Visibility.Visible;
            }
            else
            {
                ErrorDetail.SetDataPartialLOT(data);
                MachineCounters.Visibility = Visibility.Collapsed;
            }
        }
    }
}
