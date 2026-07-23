using MathNet.Numerics.Distributions;
using System;
using System.Linq;
using System.Windows.Controls;
using System.Windows.Forms.DataVisualization.Charting;
using static ClassLibrary.BasicTypes;

namespace TSMS_Vis.UserControls
{
    /// <summary>
    /// Interaction logic for UC_Probability.xaml
    /// </summary>
    public partial class UC_Probability : UserControl
    {
        public UC_Probability()
        {
            InitializeComponent();

            ChartArea chartArea = chart1.ChartAreas[0];
            chartArea.Name = "ChartArea";
            chartArea.AlignmentOrientation = AreaAlignmentOrientations.None;
            chartArea.AxisX.IntervalType = DateTimeIntervalType.Number;
            chartArea.AxisX.IsLabelAutoFit = false;
            chartArea.AxisX.IsStartedFromZero = false;
            chartArea.AxisX.LabelStyle.Format = "##0.000";
            chartArea.AxisX.LabelStyle.Interval = 0D;
            chartArea.AxisX.LabelStyle.IntervalType = DateTimeIntervalType.Number;
            chartArea.AxisX.MajorGrid.LineColor = System.Drawing.Color.Gainsboro;
            chartArea.AxisX.MajorGrid.LineDashStyle = ChartDashStyle.DashDot;
            chartArea.AxisX.ScaleView.Zoomable = false;
            chartArea.AxisX.ScrollBar.Enabled = false;
            chartArea.AxisX.ScrollBar.IsPositionedInside = false;
            chartArea.AxisY.LabelStyle.Format = "##0.00 \'%";
            chartArea.AxisY.MajorGrid.LineColor = System.Drawing.Color.Gainsboro;
            chartArea.AxisY.MajorGrid.LineDashStyle = ChartDashStyle.DashDot;
            chartArea.AxisY.ScaleBreakStyle.StartFromZero = StartFromZero.Yes;
            chartArea.AxisY.ScaleView.Zoomable = false;
            chartArea.AxisY.ScrollBar.Enabled = false;
            chartArea.AxisY.Minimum = Normal.InvCDF(0, 1, 0.000000000000001 / 100);
            chartArea.AxisY.Maximum = Normal.InvCDF(0, 1, 99.99999999999999 / 100);

            chart1.Series[0].Name = "Series1";
            chart1.Series[0].ChartArea = "ChartArea";
            chart1.Series[0].ChartType = SeriesChartType.FastPoint;
            chart1.Series[0].BorderDashStyle = ChartDashStyle.NotSet;
            chart1.Series[0].BorderWidth = 0;
            chart1.Series[0].Color = System.Drawing.Color.Red;
            chart1.Series[0].MarkerSize = 3;
            chart1.Series[0].MarkerStyle = MarkerStyle.Circle;

            chart1.Series[1].Name = "Series2";
            chart1.Series[1].ChartArea = "ChartArea";
            chart1.Series[1].ChartType = SeriesChartType.FastLine;
            chart1.Series[1].Color = System.Drawing.Color.Blue;

            chart1.Series[2].Name = "Series3";
            chart1.Series[2].ChartArea = "ChartArea";
            chart1.Series[2].ChartType = SeriesChartType.FastLine;
            chart1.Series[2].Color = System.Drawing.Color.Gold;

            chart1.Series[3].Name = "Series4";
            chart1.Series[3].ChartArea = "ChartArea";
            chart1.Series[3].ChartType = SeriesChartType.FastLine;
            chart1.Series[3].Color = System.Drawing.Color.Gold;

            chart1.Series[4].Name = "Series5";
            chart1.Series[4].ChartArea = "ChartArea";
            chart1.Series[4].ChartType = SeriesChartType.FastLine;
            chart1.Series[4].Color = System.Drawing.Color.DodgerBlue;

            chart1.Series[5].Name = "Series6";
            chart1.Series[5].ChartArea = "ChartArea";
            chart1.Series[5].ChartType = SeriesChartType.FastLine;
            chart1.Series[5].Color = System.Drawing.Color.DodgerBlue;
        }

        void ResetProbabilityCharts(bool resetTitle = false)
        {
            chart1.ChartAreas[0].AxisY.CustomLabels.Clear();
            double[] yLabels = new double[] { 0.000000000000001, 0.01, 1, 5, 20, 50, 80, 95, 99, 99.99 };

            for (int i = 0; i < yLabels.Count(); ++i)
            {
                var nextElelemnt = i + 1 == yLabels.Count() ? 99.99999999999999 : yLabels[i + 1];
                chart1.ChartAreas[0].AxisY.CustomLabels.Add
                    (
                        new CustomLabel()
                        {
                            FromPosition = Normal.InvCDF(0, 1, yLabels[i] / 100),
                            ToPosition = Normal.InvCDF(0, 1, nextElelemnt / 100),
                            Text = (yLabels[i]).ToString("0.00 '%"),
                            GridTicks = GridTickTypes.All
                        }
                    );
            }

            chart1.ChartAreas[0].AxisX.LabelStyle.Format = "###0.00";

            chart1.ChartAreas[0].AxisY.Minimum = Normal.InvCDF(0, 1, 0.000000000000001 / 100);
            chart1.ChartAreas[0].AxisY.Maximum = Normal.InvCDF(0, 1, 99.99999999999999 / 100);

            chart1.Series
               .Select(item => item.Points)
               .ToList()
               .ForEach(point => point.Clear());

            if (resetTitle) 
                chart1.Titles.Clear();
        }

        public bool DrawProbabilityPlot(string title, ProbabilityData data)
        {
            lTitle.Content = title;

            ResetProbabilityCharts();

            try
            {
                foreach (var series in chart1.Series)
                {
                    series.Points.Clear();
                }

                var probabilityPlotXYvalues = data.Points;

                if (probabilityPlotXYvalues.Count() == 0)
                    return false;

                var xAxisValues = probabilityPlotXYvalues.Select(value => value.X).ToArray();
                var yAxisValues = probabilityPlotXYvalues.Select(value => value.Y).ToArray();

                var yAxisValueMin = yAxisValues.Min();
                var yAxisValueMax = yAxisValues.Max();

                //Filter too high and too low value pairs.
                //bestFitLinevalues = bestFitLinevalues.Where(value => (value.Y <= yAxisValueMax) && (value.Y >= yAxisValueMin)).ToList();

                var bestFitLinevalues = data.BestFitLine;

                var xAxisExpectedPoints = bestFitLinevalues.Select(value => value.X).ToArray();
                var yAxisExpectedPoints = bestFitLinevalues.Select(value => value.Y).ToArray();

                var yAxisExpectedPointsMin = yAxisExpectedPoints.Min();
                var yAxisExpectedPointsMax = yAxisExpectedPoints.Max();

                chart1.Series[0].Points.DataBindXY(xAxisValues, yAxisValues);
                chart1.Series[1].Points.DataBindXY(xAxisExpectedPoints, yAxisExpectedPoints);

                //Narrowed Thresholds ...
                chart1.Series[2].Points.DataBindXY(new double[] { data.CurrentThresholdMin, data.CurrentThresholdMin }, new double[] { Math.Min(yAxisValueMin, yAxisExpectedPointsMin), Math.Max(yAxisValueMax, yAxisExpectedPointsMax) });
                chart1.Series[3].Points.DataBindXY(new double[] { data.CurrentThresholdMax, data.CurrentThresholdMax }, new double[] { Math.Min(yAxisValueMin, yAxisExpectedPointsMin), Math.Max(yAxisValueMax, yAxisExpectedPointsMax) });

                ////BV Thresholds ...
                chart1.Series[4].Points.DataBindXY(new double[] { data.BV_ThresholdMin, data.BV_ThresholdMin }, new double[] { Math.Min(yAxisValueMin, yAxisExpectedPointsMin), Math.Max(yAxisValueMax, yAxisExpectedPointsMax) });
                chart1.Series[5].Points.DataBindXY(new double[] { data.BV_ThresholdMax, data.BV_ThresholdMax }, new double[] { Math.Min(yAxisValueMin, yAxisExpectedPointsMin), Math.Max(yAxisValueMax, yAxisExpectedPointsMax) });

                chart1.ChartAreas[0].AxisX.Minimum = data.BV_ThresholdMin;//xAxisValues.Min();
                chart1.ChartAreas[0].AxisX.Maximum = data.BV_ThresholdMax; // xAxisValues.Max();

                return true;
            }
            catch (Exception)
            {
                return false;
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
