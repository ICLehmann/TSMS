using System;
using System.Collections.Generic;

namespace ClassLibrary
{
    public static class BasicTypes
    {
                 
        public struct HistogramRanges
        {
            public HistogramChartDataPoint DataPoints;
            public HistogramChartRange DataRange;
        }

        public struct HistogramChartDataPoint
        {
            public double X;
            public double Y;
        }

        public struct HistogramChartRange
        {
            public double Percentage;
            public double Min;
            public double Max;
        }


        #region classes

        public class HistogramData : IDisposable
        {
            public List<HistogramIntervals> Intervals;
            public double CurrentThresholdMin;
            public double CurrentThresholdMax;
            public double BV_ThresholdMin;
            public double BV_ThresholdMax;
            public bool PATnarrowing;
            public int StartPart;
            public List<HistogramRanges> RangeData;

            #region IDisposable Support
            private bool disposedValue = false; // To detect redundant calls

            protected virtual void Dispose(bool disposing)
            {
                if (!disposedValue)
                {
                    if (disposing)
                    {

                    }
                    Intervals.Clear();
                    RangeData.Clear();

                    disposedValue = true;
                }
            }

            // This code added to correctly implement the disposable pattern.
            public void Dispose()
            {
                Dispose(true);
                GC.SuppressFinalize(this);
            }
            #endregion
        }

        public class ProbabilityData
        {
            public List<ChartXYPoint> Points;
            public List<ChartXYPoint> BestFitLine;
            public double CurrentThresholdMin;
            public double CurrentThresholdMax;
            public double BV_ThresholdMin;
            public double BV_ThresholdMax;
        }

        public class ChartXYPoint
        {
            public double X;
            public double Y;
        }

        public class HistogramIntervals
        {
            public int IntervalIndex;
            public double IntervalWidth;
            public double Min;
            public double Max;
        }

        #endregion classes
    }
}
