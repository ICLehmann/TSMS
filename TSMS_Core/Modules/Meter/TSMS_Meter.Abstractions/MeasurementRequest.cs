namespace TSMS.Meter.Abstractions;

public sealed class MeasurementRequest
{
    public string CorrelationId { get; init; } = string.Empty;
    public string? LotId { get; init; }
    public string Pattern { get; init; } = string.Empty;
    public List<MeterDeviceRequest> Devices { get; } = new();
}

public sealed class MeterDeviceRequest
{
    public string DeviceId { get; init; } = string.Empty;
    public MeterDeviceType DeviceType { get; init; }
    public int StationNumber { get; init; }
    public int TriggerDelayMs { get; init; }
    public List<MeterChannelRequest> Channels { get; } = new();
}

public sealed class MeterChannelRequest
{
    public string Name { get; init; } = string.Empty;
    public double Min { get; init; }
    public double Nominal { get; init; }
    public double Max { get; init; }
    public bool PatEnabled { get; init; }
    public double PatMin { get; init; }
    public double PatMax { get; init; }
}

public enum MeterDeviceType
{
    Isolation = 0,
    InductanceImpedance = 1,
    Capacity = 2,
    Resistance = 3
}
