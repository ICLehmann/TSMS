namespace TSMS.Core.GuiUdp;

public sealed class GuiUdpOutputOptions
{
    public bool Enabled { get; set; } = true;
    public string IpAddress { get; set; } = "127.0.0.1";
    public int SendPort { get; set; } = 8500;
    public string MachineName { get; set; } = "TSMS";
}
