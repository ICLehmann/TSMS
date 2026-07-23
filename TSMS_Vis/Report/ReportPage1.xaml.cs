using System.Windows;

namespace TSMS_Vis.Report
{
    public partial class ReportPage1 : Window
    {
        public ReportPage1(ReportData data, int page, int num_pages)
        {
            InitializeComponent();
            Header.SetPage(page, num_pages);
            Header.SetLotData(data);
            Header.SetWarning(false);

            MeasTable.SetData(data);

            // depends on report type (full or narrowed section)
            if (data.reportType.CompleteLOT == false)
            {
                ErrorSummary.SetDataPartialLOT(data);
                Narrowing.SetData(data);
                Narrowing.Visibility = Visibility.Visible;
                NarrowingSections.Visibility = Visibility.Collapsed;
            }
            else
            {
                ErrorSummary.SetDataCompleteLOT(data);
                NarrowingSections.SetData(data);
                NarrowingSections.Visibility = Visibility.Visible;
                Narrowing.Visibility = Visibility.Collapsed;
            }
        }
    }
}
