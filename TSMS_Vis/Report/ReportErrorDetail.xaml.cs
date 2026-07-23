using System;
using System.Windows.Controls;
using TSMS_Vis.Resources;

namespace TSMS_Vis.Report
{
    public partial class ReportErrorDetail : UserControl
    {

        public class ErrorDetail
        {
            public string Name { get; set; }
            public string Num { get; set; }
            public string Percent { get; set; }
            public string Hint { get; set; }
        }

        public ReportErrorDetail()
        {
            InitializeComponent();
        }

        public void SetDataCompleteLOT(ReportData data)
        {
            double totalparts = data.MachineCntTotal;

            lvErrorDetails.Items.Clear();

            for (int i = 0; i < data.measData.Count; i++)
            {
                if (data.measData[i].Name == "Z2" || data.measData[i].Name == "Z3" ||
                    data.measData[i].Name == "C2" || data.measData[i].Name == "C3")
                    continue;

                double percentHigh = data.measData[i].CntHigh / totalparts * 100.0;
                double percentLow = data.measData[i].CntLow / totalparts * 100.0;
                lvErrorDetails.Items.Add(
                    new ErrorDetail() 
                    {
                        Name = data.measData[i].Name + " " + ReportRes.HIGH_LC, 
                        Num = data.measData[i].CntHigh.ToString(), 
                        Percent = Math.Round(percentHigh, 3).ToString() + " %" 
                    });
                lvErrorDetails.Items.Add(
                    new ErrorDetail()
                    {
                        Name = data.measData[i].Name + " " + ReportRes.LOW_LC,
                        Num = data.measData[i].CntLow.ToString(),
                        Percent = Math.Round(percentLow, 3).ToString() + " %" 
                    });
            }

            // add system failure
            for (int i = 0; i < data.measData.Count; i++)
            {
                if (data.measData[i].Name == "Z2" || data.measData[i].Name == "Z3" ||
                    data.measData[i].Name == "C2" || data.measData[i].Name == "C3")
                    continue;

                double percentSystem = data.measData[i].CntFail / totalparts * 100.0;
                lvErrorDetails.Items.Add(
                    new ErrorDetail() 
                    {
                        Name = "System " + data.measData[i].Name,
                        Num = data.measData[i].CntFail.ToString(),
                        Percent = Math.Round(percentSystem, 3).ToString() + " %"
                    });    
            }
        }

        public void SetDataPartialLOT(ReportData data)
        {
            double totalparts = data.Section_TotalParts; // changed from MachineCntTotal

            lvErrorDetails.Items.Clear();

            for (int i = 0; i < data.measData.Count; i++)
            {
                if (data.measData[i].Name == "Z2" || data.measData[i].Name == "Z3" ||
                    data.measData[i].Name == "C2" || data.measData[i].Name == "C3")
                    continue;

                double percentHigh = data.measData[i].Section_CntHigh / totalparts * 100.0;
                double percentLow = data.measData[i].Section_CntLow / totalparts * 100.0;
                lvErrorDetails.Items.Add(
                    new ErrorDetail()
                    {
                        Name = data.measData[i].Name + " " + ReportRes.HIGH_LC,
                        Num = data.measData[i].Section_CntHigh.ToString(),
                        Percent = Math.Round(percentHigh, 3).ToString() + " %"
                    });
                lvErrorDetails.Items.Add(
                    new ErrorDetail()
                    {
                        Name = data.measData[i].Name + " " + ReportRes.LOW_LC,
                        Num = data.measData[i].Section_CntLow.ToString(),
                        Percent = Math.Round(percentLow, 3).ToString() + " %"
                    });
            }

            // add system failure
            for (int i = 0; i < data.measData.Count; i++)
            {
                if (data.measData[i].Name == "Z2" || data.measData[i].Name == "Z3" ||
                    data.measData[i].Name == "C2" || data.measData[i].Name == "C3")
                    continue;

                double percentSystem = data.measData[i].Section_CntFail / totalparts * 100.0;
                lvErrorDetails.Items.Add(
                    new ErrorDetail()
                    {
                        Name = "System " + data.measData[i].Name,
                        Num = data.measData[i].Section_CntFail.ToString(),
                        Percent = Math.Round(percentSystem, 3).ToString() + " %"
                    });
            }
        }
    }
}
