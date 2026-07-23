using TSMS.Core;

internal static class Program
{
    private static async Task<int> Main(string[] args)
    {
        // Load runtime configuration from the app output directory.
        var appBase = AppContext.BaseDirectory;
        var options = CoreOptionsLoader.Load(Path.Combine(appBase, "appsettings.json"));

        // Resolve modules by configured type names so machine and meter stay pluggable.
        var machineModule = MachineModuleFactory.Create(options.MachineModule);
        var meterModule = MeterModuleFactory.Create(options.MeterModule);
        var engine = new CoreEngine(options, machineModule, meterModule);

        Console.WriteLine($"TSMS_Core started. Profile: {options.Profile.Name}, machine: {options.MachineModule.TypeName}, meter: {options.MeterModule.TypeName}");
        Console.WriteLine("Press Ctrl+C to stop.");

        using var cts = new CancellationTokenSource();
        Console.CancelKeyPress += (_, eventArgs) =>
        {
            eventArgs.Cancel = true;
            // Trigger graceful shutdown instead of hard process termination.
            cts.Cancel();
        };

        await engine.RunAsync(cts.Token);
        return 0;
    }
}
