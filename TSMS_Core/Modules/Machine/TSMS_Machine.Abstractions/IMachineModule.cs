namespace TSMS.Machine.Abstractions;

public interface IMachineModule
{
    event EventHandler<MachineTrigger>? TriggerReceived;

    Task InitializeAsync(MachineModuleSettings settings, CancellationToken cancellationToken);
    Task StartAsync(CancellationToken cancellationToken);
    Task PublishResultAsync(CoreResult result, CancellationToken cancellationToken);
    Task StopAsync(CancellationToken cancellationToken);
}
