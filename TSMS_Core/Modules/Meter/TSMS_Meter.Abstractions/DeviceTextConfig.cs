namespace TSMS.Meter.Abstractions;

public sealed class DeviceTextConfig
{
    public string DeviceName { get; init; } = string.Empty;
    public bool IsGpib { get; set; }
    public string Address { get; set; } = string.Empty;
    public int PortOrDevice { get; set; }
    public int StationNumber { get; set; }
    public int TriggerDelayMs { get; set; }
    public int WaitAfterStartMs { get; set; }
    public List<DeviceValueDefinition> Values { get; } = new();
    public List<string> InitializationSequence { get; } = new();
    public List<string> SetupSequence { get; } = new();
    public List<string> StartMeasurementSequence { get; } = new();
    public List<string> GetAnswerSequence { get; } = new();
    public List<string> CompensationInitSequence { get; } = new();
    public List<string> CompensationOpenSequence { get; } = new();
    public List<string> CompensationShortSequence { get; } = new();
}

public sealed class DeviceValueDefinition
{
    public string Name { get; set; } = string.Empty;
    public string Unit { get; set; } = string.Empty;
    public int Position { get; set; }
}
