using System;
using System.Windows.Controls;
using TSMS_Vis.Resources;

namespace TSMS_Vis.Report
{
    /// <summary>
    /// Interaction logic for ReportErrorSummary.xaml
    /// </summary>
    public partial class ReportErrorSummary : UserControl
    {
        public class ErrorSummary
        {
            public string Name { get; set; }
            public string Num { get; set; }
            public string Percent { get; set; }
        }

        public ReportErrorSummary()
        {
            InitializeComponent();
        }
        public void SetDataCompleteLOT(ReportData data)
        {
            double totalparts = data.MachineCntTotal;

            lvErrorSummary.Items.Clear();
            lvErrorSummary.Items.Add(
                new ErrorSummary()
                {
                    Name = ReportRes.TOTAL,
                    Num = totalparts.ToString(),
                    Percent = "100 %"
                });
            double percent;
            double passedPercent = data.MachineCntGood / totalparts * 100.0;
            lvErrorSummary.Items.Add(
                new ErrorSummary()
                {
                    Name = ReportRes.QUAL_PASSED,
                    Num = data.MachineCntGood.ToString(),
                    Percent = Math.Round(passedPercent, 3).ToString() + " %"
                });
            percent = 100 - passedPercent;
            lvErrorSummary.Items.Add(
                new ErrorSummary()
                {
                    Name = ReportRes.QUAL_FAILED,
                    Num = (totalparts - data.MachineCntGood).ToString(),
                    Percent = Math.Round(percent, 3).ToString() + " %" 
                });
            percent = data.Section_TotalFailed / totalparts * 100.0; // changed from CounterElFail
            lvErrorSummary.Items.Add(
                new ErrorSummary()
                {
                    Name = ReportRes.QUAL_EL_FAILED,
                    Num = data.Section_TotalFailed.ToString(), // changed from CounterElFail
                    Percent = Math.Round(percent, 3).ToString() + " %"
                });
            percent = data.Section_TotalSysFailed / totalparts * 100.0; // changed from CounterSysErr
            lvErrorSummary.Items.Add(
                new ErrorSummary()
                {
                    Name = ReportRes.QUAL_EL_SYSTEM_FAILED,
                    Num = data.Section_TotalSysFailed.ToString(), // changed from CounterSysErr
                    Percent = Math.Round(percent, 3).ToString() + " %"
                });

            int AOI_sum = 0;
            foreach (int cnt in data.CounterVRB)
                AOI_sum += cnt;

            percent = AOI_sum / totalparts * 100.0;
            lvErrorSummary.Items.Add(
                new ErrorSummary()
                {
                    Name = ReportRes.QUAL_AOI_FAILED_TOTAL,
                    Num = AOI_sum.ToString(),
                    Percent = Math.Round(percent, 3).ToString() + " %"
                }); ;
        }

        public void SetDataPartialLOT(ReportData data)
        {
            double totalparts = data.Section_TotalParts;

            lvErrorSummary.Items.Clear();
            lvErrorSummary.Items.Add(
                new ErrorSummary()
                {
                    Name = ReportRes.EL_TOTAL,
                    Num = totalparts.ToString(),
                    Percent = "100 %"
                });
            double percent;
            percent = data.Section_TotalGoods / totalparts * 100.0;
            lvErrorSummary.Items.Add(
                new ErrorSummary()
                {
                    Name = ReportRes.QUAL_EL_PASSED,
                    Num = data.Section_TotalGoods.ToString(),
                    Percent = Math.Round(percent, 3).ToString() + " %"
                });
            percent = data.Section_TotalFailed / totalparts * 100.0;
            lvErrorSummary.Items.Add(
                new ErrorSummary()
                {
                    Name = ReportRes.QUAL_EL_FAILED,
                    Num = data.Section_TotalFailed.ToString(),
                    Percent = Math.Round(percent, 3).ToString() + " %"
                });
            percent = data.Section_TotalSysFailed / totalparts * 100.0;
            lvErrorSummary.Items.Add(
                new ErrorSummary()
                {
                    Name = ReportRes.QUAL_EL_SYSTEM_FAILED,
                    Num = data.Section_TotalSysFailed.ToString(),
                    Percent = Math.Round(percent, 3).ToString() + " %"
                });
        }
    }
}
