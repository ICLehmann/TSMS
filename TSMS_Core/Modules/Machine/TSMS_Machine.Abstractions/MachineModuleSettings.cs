namespace TSMS.Machine.Abstractions;

public sealed class MachineModuleSettings
{
    public string MachineId { get; set; } = string.Empty;
    public Dictionary<string, string> Parameters { get; set; } = new(StringComparer.OrdinalIgnoreCase);
}
