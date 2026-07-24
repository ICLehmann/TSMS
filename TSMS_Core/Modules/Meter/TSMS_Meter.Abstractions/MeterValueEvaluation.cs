using System.Globalization;

namespace TSMS.Meter.Abstractions;

public static class MeterValueEvaluation
{
    // Keep legacy FAIL threshold: values far outside tolerance are hard failures.
    private const double FailFactor = 10.0;

    public static string Evaluate(double value, MeterChannelRequest channel)
    {
        if (channel.Max != 0 && value > channel.Max)
        {
            return value > channel.Max * FailFactor ? "FAIL" : "HIGH";
        }

        if (channel.Min != 0 && value < channel.Min)
        {
            return value < channel.Min / FailFactor ? "FAIL" : "LOW";
        }

        if (channel.PatEnabled)
        {
            if (channel.PatMax != 0 && value > channel.PatMax)
            {
                return "HIGH";
            }

            if (channel.PatMin != 0 && value < channel.PatMin)
            {
                return "LOW";
            }
        }

        return "PASS";
    }

    public static DeviceMeasurementResult CreateFailureResult(MeterDeviceRequest request, string errorMessage, int durationMs)
    {
        // Emit one FAIL point per requested channel so downstream logic stays deterministic.
        var result = new DeviceMeasurementResult
        {
            DeviceId = request.DeviceId,
            DeviceType = request.DeviceType,
            DurationMs = durationMs,
            ErrorMessage = errorMessage
        };

        foreach (var channel in request.Channels)
        {
            result.Points.Add(new MeasurementPoint
            {
                DeviceId = request.DeviceId,
                DeviceType = request.DeviceType,
                Name = channel.Name,
                Value = 0.0,
                Evaluation = "FAIL",
                StationNumber = request.StationNumber,
                TimeMs = durationMs,
                PatEnabled = channel.PatEnabled,
                PatMin = channel.PatMin,
                PatMax = channel.PatMax
            });
        }

        return result;
    }

    public static IReadOnlyList<double> ParseCsvValues(string text)
    {
        // TCP/GPIB devices typically return comma separated values in channel order.
        var values = new List<double>();
        var parts = text.Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
        foreach (var part in parts)
        {
            if (double.TryParse(part, NumberStyles.Float, CultureInfo.InvariantCulture, out var parsed))
            {
                values.Add(parsed);
            }
        }

        return values;
    }
}
