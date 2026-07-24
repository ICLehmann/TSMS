using TSMS.Meter.Abstractions;

namespace TSMS.Meter.Sim;

public sealed class SimMeterDeviceModule : IMeterDeviceModule
{
    private readonly Dictionary<string, SimulatedMeterDevice> _devices = new(StringComparer.OrdinalIgnoreCase);
    private MeterModuleSettings _settings = new();

    public string TransportName => "SIM";

    public Task InitializeAsync(MeterModuleSettings settings, CancellationToken cancellationToken)
    {
        _settings = settings;
        _devices.Clear();
        return Task.CompletedTask;
    }

    public Task StopAsync(CancellationToken cancellationToken)
    {
        _devices.Clear();
        return Task.CompletedTask;
    }

    public Task<DeviceMeasurementResult> MeasureDeviceAsync(
        MeterDeviceRequest request,
        DeviceTextConfig? deviceConfig,
        CancellationToken cancellationToken)
    {
        // Persist simulated device instances so deviations stay stable per logical device.
        var key = $"{request.DeviceId}:{request.DeviceType}";
        if (!_devices.TryGetValue(key, out var device))
        {
            var deviation = ReadDeviation(request.DeviceType);
            device = new SimulatedMeterDevice(request.DeviceId, request.DeviceType, deviation);
            _devices[key] = device;
        }

        return device.MeasureAsync(request, cancellationToken);
    }

    private int ReadDeviation(MeterDeviceType deviceType)
    {
        var defaultDeviation = ReadInt("SimulatedDeviation", 5);
        return deviceType switch
        {
            MeterDeviceType.Isolation => ReadInt("IsolationDeviation", defaultDeviation),
            MeterDeviceType.InductanceImpedance => ReadInt("ImpedanceDeviation", defaultDeviation),
            MeterDeviceType.Capacity => ReadInt("CapacityDeviation", defaultDeviation),
            MeterDeviceType.Resistance => ReadInt("ResistanceDeviation", defaultDeviation),
            _ => defaultDeviation
        };
    }

    private int ReadInt(string key, int fallback)
    {
        if (_settings.Parameters.TryGetValue(key, out var value) && int.TryParse(value, out var parsed))
        {
            return parsed;
        }

        return fallback;
    }
}
