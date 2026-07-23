using MathNet.Numerics.Distributions;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms.DataVisualization.Charting;
using static ClassLibrary.BasicTypes;

namespace ClassLibrary
{
    public static class ChartMethod
    {
        #region Histogram

        public static HistogramData CreateHistogramData(double narrowed_min, double narrowed_max, double bv_min, double bv_max, IEnumerable<double> data, bool pat, int startpart = 0)
        {
            var histData = new HistogramData
            {
                Intervals = GenerateHistogramIntervals(narrowed_min, narrowed_max),
                CurrentThresholdMin = narrowed_min,
                CurrentThresholdMax = narrowed_max,
                BV_ThresholdMin = bv_min,
                BV_ThresholdMax = bv_max,
                PATnarrowing = pat,
                StartPart = startpart
            };

            histData.RangeData = CreateHistogramPoints(histData, data);
            return histData;
        }

        private static List<HistogramIntervals> GenerateHistogramIntervals(double current_min, double current_max)
        {
            var intervals = new List<HistogramIntervals>() { };
            var binCount = 10;

            var binWidth = (current_max - current_min) / binCount;
            var binStart = current_min;
            var binEnd = current_max;

            for (int i = 5; i >= 0; --i)
            {
                intervals.Add
                (
                    new HistogramIntervals()
                    {
                        IntervalIndex = intervals.Count(),
                        IntervalWidth = binWidth,
                        Min = i == 5 ? double.NegativeInfinity : current_min - (i + 1) * binWidth,
                        Max = current_min - i * binWidth
                    }
                );
            }

            for (int i = 6; i <= 15; ++i)
            {
                intervals.Add
                (
                    new HistogramIntervals()
                    {
                        IntervalIndex = intervals.Count(),
                        IntervalWidth = binWidth,
                        Min = current_min + (i - 6) * binWidth,
                        Max = current_min + (i - 5) * binWidth,
                    }
                );
            }

            for (int i = 0; i <= 5; ++i)
            {
                intervals.Add
                (
                    new HistogramIntervals()
                    {
                        IntervalIndex = intervals.Count(),
                        IntervalWidth = binWidth,
                        Min = i == 0 ? current_max : current_max + i * binWidth,
                        Max = i == 5 ? double.PositiveInfinity : current_max + (i + 1) * binWidth
                    }
                );
            }

            return intervals.OrderByDescending(item => item.IntervalIndex).ToList();
        }

        private static List<HistogramIntervals> GenerateIntervalsBasedOnSturgesRule(List<float> data, double current_min, double current_max, double bv_min, double bv_max)
        {
            var intervals = new List<HistogramIntervals>() { };
            var binCount = SturgesRule(data);
            if (binCount == 0) return intervals;

            var binWidth = (current_max - current_min) / binCount;
            var binStart = current_min;
            var binEnd = current_max;

            intervals.Add
            (
                new HistogramIntervals()
                {
                    IntervalIndex = intervals.Count(),
                    IntervalWidth = binWidth,
                    Min = double.NegativeInfinity,
                    Max = bv_min
                }
            );

            if (bv_min != current_min)
            {
                intervals.Add
                (
                    new HistogramIntervals()
                    {
                        IntervalIndex = intervals.Count(),
                        IntervalWidth = bv_min - current_min,
                        Min = bv_min,
                        Max = current_min
                    }
                );
            }

            for (int i = 0; i < binCount; ++i)
            {
                intervals.Add
                (
                    new HistogramIntervals()
                    {
                        IntervalIndex = intervals.Count(),
                        IntervalWidth = binWidth,
                        Min = binStart + (i * binWidth),
                        Max = binStart + ((i + 1) * binWidth)
                    }
                );
            }

            if (bv_max != current_max)
            {
                intervals.Add
                (
                    new HistogramIntervals()
                    {
                        IntervalIndex = intervals.Count(),
                        IntervalWidth = bv_max - current_max,
                        Min = current_max,
                        Max = bv_max
                    }
                );
            }

            intervals.Add
            (
                new HistogramIntervals()
                {
                    IntervalIndex = intervals.Count(),
                    IntervalWidth = binWidth,
                    Min = bv_max,
                    Max = double.PositiveInfinity
                }
            );

            return intervals
                .OrderBy(item => item.IntervalIndex)
                .ToList();
        }

        private static List<HistogramRanges> CreateHistogramPoints(HistogramData basicHistogramData, IEnumerable<double> values)
        {
            var rangeData = new List<HistogramRanges>() { };
            var sumValue = values.Count();

            for (int i = 0; i < basicHistogramData.Intervals.Count(); ++i)
            {
                var min = basicHistogramData.Intervals.ElementAt(i).Min;
                var max = basicHistogramData.Intervals.ElementAt(i).Max;
                var binWidth = basicHistogramData.Intervals.ElementAt(i).IntervalWidth;

                var dataCount = values
                    .Where(item => item > min && item <= max)
                    .Count();

                rangeData.Add
                (
                    new HistogramRanges
                    {
                        DataPoints = new HistogramChartDataPoint
                        {
                            X = min == double.NegativeInfinity
                            ? max - binWidth / 2
                            : min + binWidth / 2,
                            Y = dataCount,
                        },
                        DataRange = new HistogramChartRange
                        {
                            Percentage = sumValue == 0 ? 0 : (double)dataCount / sumValue,
                            Min = min,
                            Max = max
                        }
                    }
                );
            }

            return rangeData;
        }

        private static double Minimum(IEnumerable<float> values)
        {
            return values.Count() != 0 ? values.Min() * 0.95 : 0;
        }

        private static double Maximum(IEnumerable<float> values)
        {
            return values.Count() != 0 ? values.Max() * 1.05 : 0;
        }

        //Sturge’s Rule: A Method for Selecting the Number of Bins in a Histogram
        private static int SturgesRule(IEnumerable<float> values)
        {
            var n = values.Count();
            if (n == 0) return 0;
            return (int)Math.Round(1 + 3.322 * Math.Log10(n), 0);
        }

        private static double MinWidthOfHistogramBin(IEnumerable<float> values)
        {
            var k = SturgesRule(values);
            if (k == 0) return 0;
            return (Maximum(values) - Minimum(values)) / k;
        }

        #endregion Histogram


        public static void ResetProbabilityCharts(Chart chart, bool resetTitle = false)
        {
            chart.ChartAreas[0].AxisY.CustomLabels.Clear();
            double[] yLabels = new double[] { 0.000000000000001, 0.01, 1, 5, 20, 50, 80, 95, 99, 99.99 };

            for (int i = 0; i < yLabels.Count(); ++i)
            {
                var nextElelemnt = i + 1 == yLabels.Count() ? 99.99999999999999 : yLabels[i + 1];
                chart.ChartAreas[0].AxisY.CustomLabels.Add
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

            chart.ChartAreas[0].AxisX.LabelStyle.Format = "###0.00000";

            chart.ChartAreas[0].AxisY.Minimum = Normal.InvCDF(0, 1, 0.000000000000001 / 100);
            chart.ChartAreas[0].AxisY.Maximum = Normal.InvCDF(0, 1, 99.99999999999999 / 100);

            DeleteTrendChartPoints(chart);
            if (resetTitle) chart.Titles.Clear();
        }


        public static ProbabilityData CreateProbabilityPoints(double Narrowed_Min, double Narrowed_Max, double Min, double Max, List<double> data)
        {
            var n_min = Math.Round(Narrowed_Min, 4);
            var n_max = Math.Round(Narrowed_Max, 4);

            var probabilityPoints = GenerateProbabilityDataPoints(data);

            //var LineaerBestFitLine = GenerateLinearBestFit(probabilityPointsL, out double slopeL, out double yInterceptL);

            var LinearBestFitLine = LinearRegression(probabilityPoints);

            return new ProbabilityData
            {
                Points = probabilityPoints,
                BestFitLine = LinearBestFitLine,
                CurrentThresholdMin = n_min,
                CurrentThresholdMax = n_max,
                BV_ThresholdMin = Min,
                BV_ThresholdMax = Max
            };
        }


        private static void DeleteTrendChartPoints(Chart actualChart)
        {
            actualChart.Series
                .Select(item => item.Points)
                .ToList()
                .ForEach(point => point.Clear());
        }


        private static void SetChartAxisX(Chart chart, double shiftPercentage = 0.1)
        {
            var points = chart.Series.SelectMany(item => item.Points).ToList();
            if (points.Count() == 0) return;

            var minValue = points.Select(item => item.XValue).Min();
            var maxValue = points.Select(item => item.XValue).Max();

            var shiftValue = (maxValue - minValue) * shiftPercentage;
            minValue -= shiftValue;
            maxValue += shiftValue;

            if (maxValue <= minValue) return;

            chart.ChartAreas[0].AxisX.Minimum = minValue;
            chart.ChartAreas[0].AxisX.Maximum = maxValue;

            chart.ChartAreas[0].RecalculateAxesScale();
        }

        private static int? GetIndexOfValueInChart(Series series, int sequenceNumber)
        {
            return series.Points
                .Cast<DataPoint>()
                .Select((item, index) => new { Index = (int?)index, SequenceNumber = item.YValues.ElementAt(1) })
                .Where(item => item.SequenceNumber == sequenceNumber)
                .Select(item => item.Index)
                .FirstOrDefault();
        }

        private static LegendItem GenerateLegendItem(Color color, string legendTitle)
        {
            LegendItem item1 = new LegendItem
            {
                ImageStyle = LegendImageStyle.Rectangle,
                Color = color,
                BorderColor = color
            };
            item1.Cells.Add(LegendCellType.SeriesSymbol, "", ContentAlignment.MiddleCenter);
            item1.Cells.Add(LegendCellType.Text, legendTitle, ContentAlignment.MiddleLeft);

            return item1;
        }

        private static List<ChartXYPoint> GenerateProbabilityDataPoints(List<double> measuredValues)
        {
            var count = measuredValues.Count;

            if (count != 0)
            {
                //InvCDF ==> mean = 0 stdDev = 1
                measuredValues.Sort();
                var InverseCDF = measuredValues.Select
                (
                    (item, index) =>
                        new ChartXYPoint
                        {
                            X = item,
                            Y = Normal.InvCDF(0, 1, index + 1 == count
                                ? Math.Pow(0.5, (double)1 / count)
                                : index + 1 == 1
                                    ? 1 - (Math.Pow(0.5, (double)1 / count))
                                    : (index + 1 - 0.3175) / (count + 0.365))
                        }
                )
                .ToList();

                return InverseCDF;
            }
            else
            {
                return new List<ChartXYPoint>() { };
            }
        }

        private static List<ChartXYPoint> GenerateLinearBestFit(List<ChartXYPoint> points, out double a, out double b)
        {
            int numPoints = points.Count;
            a = 0;
            b = 0;
            if (numPoints == 0) return new List<ChartXYPoint>();

            double meanX = points.Average(point => point.X);
            double meanY = points.Average(point => point.Y);

            double sumXSquared = points.Sum(point => point.X * point.X);
            double sumXY = points.Sum(point => point.X * point.Y);

            a = (sumXY / numPoints - meanX * meanY) / (sumXSquared / numPoints - meanX * meanX);
            //y = ax + b
            //b = (a * meanX - meanY);
            b = meanY - a * meanX;

            double a1 = a;
            double b1 = b;

            return points.Select(point => new ChartXYPoint() { X = point.X, Y = a1 * point.X + b1 }).ToList();
        }

        private static List<ChartXYPoint> LinearRegression(List<ChartXYPoint> points)
        {
            if (points.Count() == 0) return new List<ChartXYPoint>();

            int numPoints = points.Count;
            var sumX = points.Sum(item => item.X);
            var sumY = points.Sum(item => item.Y);
            var sumXY = points.Sum(item => item.X * item.Y);
            var squareX = points.Sum(item => item.X * item.X);

            var m = (numPoints * sumXY - sumX * sumY) / (numPoints * squareX - Math.Pow(sumX, 2));

            var b = (sumY - m * sumX) / numPoints;

            var min = Normal.InvCDF(0, 1, 0.000000000000001 / 100);
            var max = Normal.InvCDF(0, 1, 99.99999999999999 / 100);

            //y = m * x + b
            return points
                .Select
                (
                    item =>
                    new ChartXYPoint
                    {
                        X = item.X,
                        Y = m * item.X + b
                    }
                )
                .Where(item => item.Y >= min && item.Y <= max)
                .ToList();
        }
    }
}
