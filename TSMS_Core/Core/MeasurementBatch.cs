using TSMS.Meter.Abstractions;
using TSMS.Machine.Abstractions;

namespace TSMS.Core;

public sealed class MeasurementBatch
{
    public Dictionary<string, string?> Values { get; } = new(StringComparer.OrdinalIgnoreCase);
    public Dictionary<string, double> NumericValues { get; } = new(StringComparer.OrdinalIgnoreCase);

    public static MeasurementBatch FromTrigger(MachineTrigger trigger)
    {
        var batch = new MeasurementBatch();
        foreach (var entry in trigger.Data)
        {
            batch.Values[entry.Key] = entry.Value;
        }

        return batch;
    }

    public static MeasurementBatch FromMeterResponse(MeasurementResponse response)
    {
        var batch = new MeasurementBatch();

        foreach (var point in response.Points)
        {
            batch.Values[point.Name] = point.Evaluation;
            batch.NumericValues[point.Name] = point.Value;
        }

        return batch;
    }
}
