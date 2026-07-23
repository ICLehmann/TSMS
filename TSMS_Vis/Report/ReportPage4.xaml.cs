using System.Windows;

namespace TSMS_Vis.Report
{
    public partial class ReportPage4 : Window
    {
        public ReportPage4(ReportData data, int page, int num_pages)
        {
            InitializeComponent();
            Header.SetPage(page, num_pages);
            Header.SetLotData(data);
            Header.SetWarning(false);

            if (data.measData != null)
            {
                for (int i = 0; i < data.measData.Count; i++)
                {
                    if (data.measData[i].Name == "ISO")
                    {
                        var limits = new ReportLimits(data, i);
                        var dataH = ClassLibrary.ChartMethod.CreateHistogramData(limits.Narrowed_min, limits.Narrowed_max, limits.Bv_min, limits.Bv_max, data.values[i], false);
                        var title = data.measData[i].Name + " in " + data.measData[i].Unit;
                        hist1.DrawHistogram(title, dataH);
                    }

                    if (data.measData[i].Name == "L")
                    {
                        var limits = new ReportLimits(data, i);
                        var dataH = ClassLibrary.ChartMethod.CreateHistogramData(limits.Narrowed_min, limits.Narrowed_max, limits.Bv_min, limits.Bv_max, data.values[i], false);
                        var title = data.measData[i].Name + " in " + data.measData[i].Unit;
                        hist2.DrawHistogram(title, dataH);
                    }

                    if (data.measData[i].Name == "Z1")
                    {
                        var limits = new ReportLimits(data, i);
                        var dataH = ClassLibrary.ChartMethod.CreateHistogramData(limits.Narrowed_min, limits.Narrowed_max, limits.Bv_min, limits.Bv_max, data.values[i], false);
                        var title = data.measData[i].Name + " in " + data.measData[i].Unit;
                        hist3.DrawHistogram(title, dataH);
                    }

                    if (data.measData[i].Name == "C1")
                    {
                        var limits = new ReportLimits(data, i);
                        var dataH = ClassLibrary.ChartMethod.CreateHistogramData(limits.Narrowed_min, limits.Narrowed_max, limits.Bv_min, limits.Bv_max, data.values[i], false);
                        var title = data.measData[i].Name + " in " + data.measData[i].Unit;
                        hist4.DrawHistogram(title, dataH);
                    }
                }
            }
        }
    }
}
