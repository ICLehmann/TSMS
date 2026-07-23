using TSMS.Core.GuiUdp;

namespace TSMS.Core;

public sealed class CoreOptions
{
    public ProfileOptions Profile { get; set; } = new();
    public ModuleOptions MachineModule { get; set; } = new();
    public ModuleOptions MeterModule { get; set; } = new();
    public GuiUdpOutputOptions GuiUdp { get; set; } = new();
}

public sealed class ProfileOptions
{
    public string Name { get; set; } = "HU";
    public bool MeasureC { get; set; } = false;
    public bool AutoLabelPrint { get; set; } = true;
    public bool ReworkTracking { get; set; } = true;
    public string PatEvaluationMode { get; set; } = "CriticalOnly";
    public string LotIdFormat { get; set; } = "AlphaNumeric";
    public string AoiLayout { get; set; } = "HU";
}

public sealed class ModuleOptions
{
    public string AssemblyPath { get; set; } = string.Empty;
    public string TypeName { get; set; } = string.Empty;
    public Dictionary<string, string> Settings { get; set; } = new(StringComparer.OrdinalIgnoreCase);
}
