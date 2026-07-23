using System.Linq.Expressions;
using System.Windows;

namespace TSMS_Vis.Report
{
    public partial class ReportPage3 : Window
    {
        public ReportPage3(ReportData data, int page, int num_pages)
        {
            InitializeComponent();
            Header.SetPage(page, num_pages);
            Header.SetLotData(data);
            Header.SetWarning(false);

            if (data.measData != null)
            {
                for (int i = 0; i < data.measData.Count; i++)
                {
                    if (data.measData[i].Name == "L")
                    {
                        var limits = new ReportLimits(data, i);
                        var dataH = ClassLibrary.ChartMethod.CreateHistogramData(limits.Narrowed_min, limits.Narrowed_max, limits.Bv_min, limits.Bv_max, data.values[i], false);
                        HistTab1.PrintHistogram(dataH);
                    }

                    if (data.measData[i].Name == "Z1")
                    {
                        var limits = new ReportLimits(data, i);
                        var dataH = ClassLibrary.ChartMethod.CreateHistogramData(limits.Narrowed_min, limits.Narrowed_max, limits.Bv_min, limits.Bv_max, data.values[i], false);
                        HistTab2.PrintHistogram(dataH);
                    }

                    if (data.measData[i].Name == "R1")
                    {
                        var limits = new ReportLimits(data, i);
                        var dataH = ClassLibrary.ChartMethod.CreateHistogramData(limits.Narrowed_min, limits.Narrowed_max, limits.Bv_min, limits.Bv_max, data.values[i], false);
                        HistTab3.PrintHistogram(dataH);
                    }

                    if (data.measData[i].Name == "R2")
                    {
                        var limits = new ReportLimits(data, i);
                        var dataH = ClassLibrary.ChartMethod.CreateHistogramData(limits.Narrowed_min, limits.Narrowed_max, limits.Bv_min, limits.Bv_max, data.values[i], false);
                        HistTab4.PrintHistogram(dataH);
                    }
                }
            }
        }
    }
}
