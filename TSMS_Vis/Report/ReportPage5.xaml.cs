using System.Windows;

namespace TSMS_Vis.Report
{
    /// <summary>
    /// Interaction logic for ReportPage5.xaml
    /// </summary>
    public partial class ReportPage5 : Window
    {
        public ReportPage5(ReportData data, int page, int num_pages)
        {
            InitializeComponent();
            Header.SetPage(page, num_pages);
            Header.SetLotData(data);
            Header.SetWarning(false);

            for (int i = 0; i < data.measData.Count; i++)
            { 
                if (data.measData[i].Name == "R1")
                {
                    var limits = new ReportLimits(data, i);
                    var dataH = ClassLibrary.ChartMethod.CreateHistogramData(limits.Narrowed_min, limits.Narrowed_max, limits.Bv_min, limits.Bv_max, data.values[i], false);
                    string title = data.measData[i].Name + " in " + data.measData[i].Unit;
                    hist1.DrawHistogram(title, dataH);
                }

                if (data.measData[i].Name == "R2")
                {
                    var limits = new ReportLimits(data, i);
                    var dataH = ClassLibrary.ChartMethod.CreateHistogramData(limits.Narrowed_min, limits.Narrowed_max, limits.Bv_min, limits.Bv_max, data.values[i], false);
                    string title = data.measData[i].Name + " in " + data.measData[i].Unit;
                    hist2.DrawHistogram(title, dataH);
                }
            }

        }
    }
}
