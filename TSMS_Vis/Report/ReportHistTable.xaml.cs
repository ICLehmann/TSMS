using System.Windows.Controls;
using static ClassLibrary.BasicTypes;

namespace TSMS_Vis.Report
{
    /// <summary>
    /// Interaction logic for ReportHistTable.xaml
    /// </summary>
    public partial class ReportHistTable : UserControl
    {
        public class HistTableEntry
        {
            public string From { get; set; }
            public string To { get; set; }
            public string Items { get; set; }
            public string Percent { get; set; }
        }

        public ReportHistTable()
        {
            InitializeComponent();
        }

        public void PrintHistogram(HistogramData histData)
        {
            lvHistTable.Items.Clear();
            histData.RangeData
              .ForEach
              (
                  item =>
                  {
                      HistTableEntry entry = new HistTableEntry();
                      entry.From = item.DataRange.Min.ToString("####0.0000");
                      entry.To = item.DataRange.Max.ToString("####0.0000");
                      entry.Items = item.DataPoints.Y.ToString("#######0");
                      entry.Percent = item.DataRange.Percentage.ToString("#######0.000 %");

                      lvHistTable.Items.Add(entry);

                      if (item.DataRange.Min == histData.CurrentThresholdMin)
                      {
                          lvHistTable.Items.Add(new HistTableEntry() { From = "-----", To = "-----", Items = "-----", Percent = "-----" });
                      }
                      else if (item.DataRange.Min == histData.CurrentThresholdMax)
                      {
                          lvHistTable.Items.Add(new HistTableEntry() { From = "+++++", To = "+++++", Items = "+++++", Percent = "++++++" });
                      }
                  }
              );
        }
    }
}
