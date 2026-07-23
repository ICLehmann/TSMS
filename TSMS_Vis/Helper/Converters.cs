using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;
using TSMS_Vis.Resources;

namespace TSMS_Vis.Converters
{
    public class EndProduct_toUIString : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value == null) return Resource.NARROWING_PART_TO_END; else return value.ToString();
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new InvalidOperationException("EndProduct_toString can only be used OneWay.");
        }
    }

    public class PATMode_toUI_String : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            switch (value)
            {
                case 0: return Resource.NARROWING_MODE_NOPAT;
                case 1: return Resource.NARROWING_MODE_STDEV;
                default: return Resource.NARROWING_MODE_UNKNOWN;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new InvalidOperationException("PATMode_toUI_String can only be used OneWay.");
        }
    }

    public class EndProduct_toReportString : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value == null) return ReportRes.NARROWING_PART_TO_END; else return value.ToString();
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new InvalidOperationException("EndProduct_toString can only be used OneWay.");
        }
    }

    public class PATMode_toReport_String : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            switch (value)
            {
                case 0: return ReportRes.NARROWING_MODE_NOPAT;
                case 1: return ReportRes.NARROWING_MODE_STDEV;
                default: return ReportRes.NARROWING_MODE_UNKNOWN;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new InvalidOperationException("PATMode_toUI_String can only be used OneWay.");
        }
    }
}
