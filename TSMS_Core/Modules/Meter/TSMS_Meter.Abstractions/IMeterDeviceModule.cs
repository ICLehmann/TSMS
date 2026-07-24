namespace TSMS.Meter.Abstractions;

public interface IMeterDeviceModule
{
    string TransportName { get; }
    // Initialize once with shared meter settings before first measurement.
    Task InitializeAsync(MeterModuleSettings settings, CancellationToken cancellationToken);
    // Execute exactly one device request and return normalized measurement points.
    Task<DeviceMeasurementResult> MeasureDeviceAsync(MeterDeviceRequest request, CancellationToken cancellationToken);
    Task StopAsync(CancellationToken cancellationToken);
}
