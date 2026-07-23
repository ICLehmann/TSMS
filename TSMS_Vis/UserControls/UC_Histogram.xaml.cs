using System;
using System.Linq;
using System.Windows.Controls;
using System.Windows.Forms.DataVisualization.Charting;
using static ClassLibrary.BasicTypes;

namespace TSMS_Vis.UserControls
{
    /// <summary>
    /// Interaction logic for UC_Histogram.xaml
    /// </summary>
    public partial class UC_Histogram : UserControl
    {
        public UC_Histogram()
        {
            InitializeComponent();

            ChartArea chartArea = chart1.ChartAreas[0];
            chartArea.Name = "ChartArea";
            chartArea.AxisX.IsLabelAutoFit = false;
            chartArea.AxisX.LabelStyle.Format = "####0.00";
            chartArea.AxisX.MajorGrid.LineColor = System.Drawing.Color.Gainsboro;
            chartArea.AxisX.MajorGrid.LineDashStyle = ChartDashStyle.DashDot;
            chartArea.AxisY.IsLabelAutoFit = false;
            chartArea.AxisY.LabelStyle.Interval = 0D;
            chartArea.AxisY.LabelStyle.IntervalType = DateTimeIntervalType.Number;
            chartArea.AxisY.MajorGrid.LineColor = System.Drawing.Color.Gainsboro;
            chartArea.AxisY.MajorGrid.LineDashStyle = ChartDashStyle.DashDot;

            chart1.Series[0].Name = "Series1";
            chart1.Series[0].ChartArea = "ChartArea";
            chart1.Series[0].ChartType = SeriesChartType.Bar;
            chart1.Series[0].Color = System.Drawing.Color.Black;
            chart1.Series[0].LabelForeColor = System.Drawing.Color.Gray;
            chart1.Series[0].SmartLabelStyle.AllowOutsidePlotArea = LabelOutsidePlotAreaStyle.Yes;
            chart1.Series[0].SmartLabelStyle.CalloutStyle = LabelCalloutStyle.None;
            chart1.Series[0].SmartLabelStyle.IsOverlappedHidden = false;
            chart1.Series[0].SmartLabelStyle.MaxMovingDistance = 150D;
            chart1.Series[0].SmartLabelStyle.MovingDirection = LabelAlignmentStyles.Right;

            chart1.Series[1].Name = "Series2";
            chart1.Series[1].ChartArea = "ChartArea";
            chart1.Series[1].ChartType = SeriesChartType.Bar;
            chart1.Series[1].Color = System.Drawing.Color.FromArgb(255, 128, 0);
            chart1.Series[1].CustomProperties = "PixelPointWidth=3";

            chart1.Series[2].Name = "Series3";
            chart1.Series[2].ChartArea = "ChartArea";
            chart1.Series[2].ChartType = SeriesChartType.Bar;
            chart1.Series[2].Color = System.Drawing.Color.Blue;
            chart1.Series[2].CustomProperties = "PixelPointWidth=3";

        }

        public void DrawHistogram(string title, HistogramData histData)
        {
            lTitle.Content = title;

            var values = histData.RangeData
                .Select(item => item.DataPoints)
                .ToList();

            var xValues = values
                //.Where(item => item.Y != 0)
                .Select(item => item.X)
                .ToList();

            var yValues = values
                //.Where(item => item.Y != 0)
                .Select(item => item.Y)
                .ToList();

            var pointCount = yValues.Sum();

            if (pointCount == 0)
                return;

            try
            {
                Clear();

                chart1.Series[0].Points.DataBindXY(xValues, yValues);

                var maxyValues = yValues.Max() * 1.20;

                //draw narrowed thresholds               
                chart1.Series[1].Points.AddXY(histData.CurrentThresholdMin, maxyValues);
                chart1.Series[1].Points.AddXY(histData.CurrentThresholdMax, maxyValues);

                chart1.ChartAreas[0].AxisX.CustomLabels.Clear();

                foreach( var item in histData.Intervals.ToList())
                {
                    var label = new CustomLabel();
                    label.FromPosition = item.Max == double.PositiveInfinity ? item.Min + item.IntervalWidth * 1.5 : item.Max + item.IntervalWidth / 2;
                    label.ToPosition = item.Min == double.NegativeInfinity ? item.Max - item.IntervalWidth / 2 : item.Min + item.IntervalWidth / 2;
                    label.Text = item.Max == double.PositiveInfinity ? (item.Min + item.IntervalWidth).ToString("#####0.00") : item.Max.ToString("#####0.00");
                    label.GridTicks = GridTickTypes.All;
                    chart1.ChartAreas[0].AxisX.CustomLabels.Add(label);
                }

                chart1.ChartAreas[0].AxisX.Minimum = chart1.ChartAreas[0].AxisX.CustomLabels.Min(item => item.ToPosition);
                chart1.ChartAreas[0].AxisX.Maximum = chart1.ChartAreas[0].AxisX.CustomLabels.Max(item => item.FromPosition);

                //BV thresholds
              //  chart1.Series[2].Points.AddXY(histData.BV_ThresholdMin, maxyValues);
               // chart1.Series[2].Points.AddXY(histData.BV_ThresholdMax, maxyValues);

                //chart.Series[1].Points
                //.ToList()
                //.ForEach
                //(
                //    item => item.Label = $"{string.Format("{0:0.00000}", item.XValue)}"
                //);

                //chart.Series[2].Points
                //.ToList()
                //.ForEach
                //(
                //    item => item.Label = $"{string.Format("{0:0.00000}", item.XValue)}"
                //);
            }
            finally
            {

                chart1.Series
                    .Select(item => item.Points)
                    .ToList()
                    .ForEach(point => point.ResumeUpdates());
            }
        }

        public void Clear()
        {
            foreach (var series in chart1.Series)
            {
                series.Points.Clear();
            }
        }
    }
}
