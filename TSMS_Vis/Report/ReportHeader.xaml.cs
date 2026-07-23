using System.Diagnostics.Eventing.Reader;
using System.Windows.Controls;
using TSMS_Vis.Resources;

namespace TSMS_Vis.Report
{
    public partial class ReportHeader : UserControl
    {
        public ReportHeader()
        {
            InitializeComponent();
        }

        public void SetPage(int page, int num_pages)
        {
            lPage.Content = ReportRes.PAGE + " " + page.ToString() + " " + ReportRes.PAGE_OF + " " + num_pages.ToString();
        }

        public void SetWarning(bool warning)
        {
            if (warning)
                lWarning.Visibility = System.Windows.Visibility.Visible;
            else
                lWarning.Visibility = System.Windows.Visibility.Hidden;
        }

        public void SetLotData(ReportData data)
        {
            lLotNo.Content = data.lotHeader.LotNr.ToString();
            lPartNo.Content = data.lotHeader.ProductNum;
            lLine.Content = data.lotHeader.LineId;
            lMachine.Content = data.lotHeader.TestMachine;
            lStartTime.Content = data.lotHeader.StartTime;
            lEndTime.Content = data.lotHeader.StopTime;
            lStatus.Content = data.lotHeader.Status;

            if (data.reportType.CompleteLOT)
                lReportData.Content = ReportRes.REPORTDATA_COMPLETE_LOT;
            else
                lReportData.Content = ReportRes.REPORTDATA_PARTIAL_LOT;
        }
    }
}
