namespace TSMS.Meter.Abstractions;

public sealed class MeterModuleSettings
{
    public string MeterProfile { get; set; } = "HU";
    public Dictionary<string, string> Parameters { get; set; } = new(StringComparer.OrdinalIgnoreCase);
}
