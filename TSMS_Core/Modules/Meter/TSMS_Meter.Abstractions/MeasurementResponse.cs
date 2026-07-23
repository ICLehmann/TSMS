namespace TSMS.Meter.Abstractions;

public sealed class MeasurementResponse
{
    public string CorrelationId { get; init; } = string.Empty;
    public List<MeasurementPoint> Points { get; } = new();
    public List<DeviceMeasurementResult> Devices { get; } = new();
}

public sealed class DeviceMeasurementResult
{
    public string DeviceId { get; init; } = string.Empty;
    public MeterDeviceType DeviceType { get; init; }
    public int DurationMs { get; init; }
    public List<MeasurementPoint> Points { get; } = new();
}

public sealed class MeasurementPoint
{
    public string DeviceId { get; init; } = string.Empty;
    public MeterDeviceType DeviceType { get; init; }
    public string Name { get; init; } = string.Empty;
    public double Value { get; init; }
    public string Evaluation { get; init; } = "NONE";
    public int StationNumber { get; init; }
    public int TimeMs { get; init; }
    public bool PatEnabled { get; init; }
    public double PatMin { get; init; }
    public double PatMax { get; init; }
}
