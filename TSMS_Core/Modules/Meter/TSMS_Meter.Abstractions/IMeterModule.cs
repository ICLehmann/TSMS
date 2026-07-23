namespace TSMS.Meter.Abstractions;

public interface IMeterModule
{
    Task InitializeAsync(MeterModuleSettings settings, CancellationToken cancellationToken);
    Task<MeasurementResponse> MeasureAsync(MeasurementRequest request, CancellationToken cancellationToken);
    Task StopAsync(CancellationToken cancellationToken);
}
