using System;
using System.Windows.Controls;
using TSMS_Vis.Resources;
using static TSMS_Vis.Report.ReportErrorDetail;

namespace TSMS_Vis.Report
{
    public partial class ReportMachineCounters : UserControl
    {
        public class MachineCounter
        {
            public string Name { get; set; }
            public string Num { get; set; }
            public string Percent { get; set; }
        }

        public ReportMachineCounters()
        {
            InitializeComponent();
        }
        public void SetData(ReportData data)
        {
            double totalparts = data.MachineCntTotal; // changed from MachineCntTotal

            lvMachnineCounters.Items.Clear();
            lvMachnineCounters.Items.Add(
                new MachineCounter()
                {
                    Name = ReportRes.MACHINECNT_TOTAL,
                    Num = data.MachineCntTotal.ToString(),
                    Percent = "100 %"
                });
            double percent = data.MachineCntGood / totalparts * 100.0;
            lvMachnineCounters.Items.Add(
                new MachineCounter()
                {
                    Name = ReportRes.MACHINECNT_GOOD,
                    Num = data.MachineCntGood.ToString(),
                    Percent = Math.Round(percent, 3).ToString() + " %"
                });

            // add electrical counters
            if (data.CounterTRB.Length == 8)
            {
                percent = data.CounterTRB[0] / totalparts * 100.0;
                lvMachnineCounters.Items.Add(
                    new ErrorDetail()
                    {
                        Name = ReportRes.BIN1_TITLE,
                        Num = data.CounterTRB[0].ToString(),
                        Percent = Math.Round(percent, 3).ToString() + " %"
                    });

                percent = data.CounterTRB[1] / totalparts * 100.0;
                lvMachnineCounters.Items.Add(
                    new ErrorDetail()
                    {
                        Name = ReportRes.BIN2_TITLE,
                        Num = data.CounterTRB[1].ToString(),
                        Percent = Math.Round(percent, 3).ToString() + " %"
                    });

                percent = data.CounterTRB[2] / totalparts * 100.0;
                lvMachnineCounters.Items.Add(
                    new ErrorDetail()
                    {
                        Name = ReportRes.BIN3_TITLE,
                        Num = data.CounterTRB[2].ToString(),
                        Percent = Math.Round(percent, 3).ToString() + " %"
                    });

                percent = data.CounterTRB[3] / totalparts * 100.0;
                lvMachnineCounters.Items.Add(
                    new ErrorDetail()
                    {
                        Name = ReportRes.BIN4_TITLE,
                        Num = data.CounterTRB[3].ToString(),
                        Percent = Math.Round(percent, 3).ToString() + " %"
                    });
                percent = data.CounterTRB[4] / totalparts * 100.0;
                lvMachnineCounters.Items.Add(
                    new ErrorDetail()
                    {
                        Name = ReportRes.BIN5_TITLE,
                        Num = data.CounterTRB[4].ToString(),
                        Percent = Math.Round(percent, 3).ToString() + " %"
                    });
                percent = data.CounterTRB[5] / totalparts * 100.0;
                lvMachnineCounters.Items.Add(
                    new ErrorDetail()
                    {
                        Name = ReportRes.BIN6_TITLE,
                        Num = data.CounterTRB[5].ToString(),
                        Percent = Math.Round(percent, 3).ToString() + " %"
                    });
                percent = data.CounterTRB[6] / totalparts * 100.0;
                lvMachnineCounters.Items.Add(
                    new ErrorDetail()
                    {
                        Name = ReportRes.BIN7_TITLE,
                        Num = data.CounterTRB[6].ToString(),
                        Percent = Math.Round(percent, 3).ToString() + " %"
                    });
            }

            // add aoi counters
            if (data.CounterVRB.Length == 6)
            {
                percent = data.CounterVRB[0] / totalparts * 100.0;
                lvMachnineCounters.Items.Add(
                    new ErrorDetail()
                    {
                        Name = ReportRes.BIN9_TITLE,
                        Num = data.CounterVRB[0].ToString(),
                        Percent = Math.Round(percent, 3).ToString() + " %"
                    });

                percent = data.CounterVRB[1] / totalparts * 100.0;
                lvMachnineCounters.Items.Add(
                    new ErrorDetail()
                    {
                        Name = ReportRes.BIN10_TITLE,
                        Num = data.CounterVRB[1].ToString(),
                        Percent = Math.Round(percent, 3).ToString() + " %"
                    });

                percent = data.CounterVRB[2] / totalparts * 100.0;
                lvMachnineCounters.Items.Add(
                    new ErrorDetail()
                    {
                        Name = ReportRes.BIN11_TITLE,
                        Num = data.CounterVRB[2].ToString(),
                        Percent = Math.Round(percent, 3).ToString() + " %"
                    });

                percent = data.CounterVRB[3] / totalparts * 100.0;
                lvMachnineCounters.Items.Add(
                    new ErrorDetail()
                    {
                        Name = ReportRes.BIN12_TITLE,
                        Num = data.CounterVRB[3].ToString(),
                        Percent = Math.Round(percent, 3).ToString() + " %"
                    });

                percent = data.MachineCntGoodBin / totalparts * 100.0;
                lvMachnineCounters.Items.Add(
                    new MachineCounter()
                    {
                        Name = ReportRes.MACHINECNT_GOODBIN,
                        Num = (data.MachineCntGoodBin).ToString(),
                        Percent = Math.Round(percent, 3).ToString() + " %"
                    });
                percent = data.MachineCntLastBin / totalparts * 100.0;
                lvMachnineCounters.Items.Add(
                    new MachineCounter()
                    {
                        Name = ReportRes.MACHINECNT_LASTBIN,
                        Num = data.MachineCntLastBin.ToString(),
                        Percent = Math.Round(percent, 3).ToString() + " %"
                    });
            }
        }
    }
}
