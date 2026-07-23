using System.Windows;

namespace TSMS_Vis.Report
{
    /// <summary>
    /// Interaction logic for ReportPage6.xaml
    /// </summary>
    public partial class ReportPage6 : Window
    {
        public ReportPage6(ReportData data, int page, int num_pages)
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
                    var dataP = ClassLibrary.ChartMethod.CreateProbabilityPoints(limits.Narrowed_min, limits.Narrowed_max, limits.Bv_min, limits.Bv_max, data.values[i]);
                    string title = data.measData[i].Name + " in " + data.measData[i].Unit;
                    Probability1.DrawProbabilityPlot(title, dataP);
                }

                if (data.measData[i].Name == "R2")
                {
                    var limits = new ReportLimits(data, i);
                    var dataP = ClassLibrary.ChartMethod.CreateProbabilityPoints(limits.Narrowed_min, limits.Narrowed_max, limits.Bv_min, limits.Bv_max, data.values[i]);
                    string title = data.measData[i].Name + " in " + data.measData[i].Unit;
                    Probability2.DrawProbabilityPlot(title, dataP);
                }
            }
        }
    }
}
