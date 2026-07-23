using TSMS.Meter.Abstractions;

namespace TSMS.Meter.Sim;

public sealed class SimMeterModule : IMeterModule
{
    private readonly Dictionary<MeterDeviceType, SimulatedMeterDevice> _devices = new();
    private bool _initialized;

    public Task InitializeAsync(MeterModuleSettings settings, CancellationToken cancellationToken)
    {
        _devices.Clear();
        _devices[MeterDeviceType.Isolation] = new SimulatedMeterDevice(
            "SM7110_1",
            MeterDeviceType.Isolation,
            ReadInt(settings, "IsolationDeviation", ReadInt(settings, "SimulatedDeviation", 5)));
        _devices[MeterDeviceType.InductanceImpedance] = new SimulatedMeterDevice(
            "IM7581_1",
            MeterDeviceType.InductanceImpedance,
            ReadInt(settings, "ImpedanceDeviation", ReadInt(settings, "SimulatedDeviation", 5)));
        _devices[MeterDeviceType.Capacity] = new SimulatedMeterDevice(
            "IM7581_2",
            MeterDeviceType.Capacity,
            ReadInt(settings, "CapacityDeviation", ReadInt(settings, "SimulatedDeviation", 5)));
        _devices[MeterDeviceType.Resistance] = new SimulatedMeterDevice(
            "RES2329",
            MeterDeviceType.Resistance,
            ReadInt(settings, "ResistanceDeviation", ReadInt(settings, "SimulatedDeviation", 5)));

        _initialized = true;
        Console.WriteLine("[SimMeter] Initialized with parallel specialized devices.");
        return Task.CompletedTask;
    }

    public async Task<MeasurementResponse> MeasureAsync(MeasurementRequest request, CancellationToken cancellationToken)
    {
        if (!_initialized)
        {
            throw new InvalidOperationException("SimMeterModule is not initialized.");
        }

        var enabledDevices = request.Devices.Where(x => x.Channels.Count > 0).ToList();
        var tasks = enabledDevices.Select(device => MeasureDeviceAsync(device, cancellationToken)).ToArray();
        var results = await Task.WhenAll(tasks);

        var response = new MeasurementResponse
        {
            CorrelationId = request.CorrelationId
        };

        foreach (var deviceResult in results)
        {
            response.Devices.Add(deviceResult);
            response.Points.AddRange(deviceResult.Points);
        }

        Console.WriteLine($"[SimMeter] Pattern={request.Pattern}, devices={results.Length}, points={response.Points.Count}");
        foreach (var deviceResult in results.OrderBy(x => x.DeviceId, StringComparer.OrdinalIgnoreCase))
        {
            Console.WriteLine($"[SimMeter] Device {deviceResult.DeviceId} ({deviceResult.DeviceType}) -> {deviceResult.Points.Count} values in {deviceResult.DurationMs} ms");
        }

        return response;
    }

    public Task StopAsync(CancellationToken cancellationToken)
    {
        _initialized = false;
        _devices.Clear();
        Console.WriteLine("[SimMeter] Stopped.");
        return Task.CompletedTask;
    }

    private async Task<DeviceMeasurementResult> MeasureDeviceAsync(MeterDeviceRequest request, CancellationToken cancellationToken)
    {
        if (!_devices.TryGetValue(request.DeviceType, out var device))
        {
            throw new InvalidOperationException($"No simulated device configured for {request.DeviceType}.");
        }

        return await device.MeasureAsync(request, cancellationToken);
    }

    private static int ReadInt(MeterModuleSettings settings, string key, int defaultValue)
    {
        if (settings.Parameters.TryGetValue(key, out var value) && int.TryParse(value, out var parsed))
        {
            return parsed;
        }

        return defaultValue;
    }
}

internal sealed class SimulatedMeterDevice
{
    private readonly Random _random = new();
    private readonly object _sync = new();
    private readonly int _deviationPercent;

    public SimulatedMeterDevice(string deviceId, MeterDeviceType deviceType, int deviationPercent)
    {
        DeviceId = deviceId;
        DeviceType = deviceType;
        _deviationPercent = deviationPercent;
    }

    public string DeviceId { get; }
    public MeterDeviceType DeviceType { get; }

    public async Task<DeviceMeasurementResult> MeasureAsync(MeterDeviceRequest request, CancellationToken cancellationToken)
    {
        var started = Environment.TickCount64;
        if (request.TriggerDelayMs > 0)
        {
            await Task.Delay(request.TriggerDelayMs, cancellationToken);
        }

        var result = new DeviceMeasurementResult
        {
            DeviceId = request.DeviceId,
            DeviceType = request.DeviceType
        };

        foreach (var channel in request.Channels)
        {
            await Task.Delay(10, cancellationToken);
            var value = CreateValue(channel);
            result.Points.Add(new MeasurementPoint
            {
                DeviceId = request.DeviceId,
                DeviceType = request.DeviceType,
                Name = channel.Name,
                Value = value,
                Evaluation = Evaluate(value, channel),
                StationNumber = request.StationNumber,
                TimeMs = (int)(Environment.TickCount64 - started),
                PatEnabled = channel.PatEnabled,
                PatMin = channel.PatMin,
                PatMax = channel.PatMax
            });
        }

        result = new DeviceMeasurementResult
        {
            DeviceId = result.DeviceId,
            DeviceType = result.DeviceType,
            DurationMs = (int)(Environment.TickCount64 - started)
        }.WithPoints(result.Points);

        return result;
    }

    private double CreateValue(MeterChannelRequest channel)
    {
        var mean = string.Equals(channel.Name, "ISO", StringComparison.OrdinalIgnoreCase)
            ? channel.Min + ((channel.Max - channel.Min) / 2.0)
            : channel.Nominal;

        var stddev = Math.Abs(channel.Max - channel.Min) / 100.0 * _deviationPercent;
        if (stddev <= 0)
        {
            stddev = 0.001;
        }

        return NextGaussian(mean, stddev);
    }

    private string Evaluate(double value, MeterChannelRequest channel)
    {
        if (channel.Max != 0 && value > channel.Max)
        {
            return value > channel.Max * 10.0 ? "FAIL" : "HIGH";
        }

        if (channel.Min != 0 && value < channel.Min)
        {
            return value < channel.Min / 10.0 ? "FAIL" : "LOW";
        }

        if (channel.PatEnabled)
        {
            if (channel.PatMax != 0 && value > channel.PatMax)
            {
                return "HIGH";
            }

            if (channel.PatMin != 0 && value < channel.PatMin)
            {
                return "LOW";
            }
        }

        return "PASS";
    }

    private double NextGaussian(double mean, double stddev)
    {
        double u1;
        double u2;
        lock (_sync)
        {
            u1 = 1.0 - _random.NextDouble();
            u2 = 1.0 - _random.NextDouble();
        }

        var randStdNormal = Math.Sqrt(-2.0 * Math.Log(u1)) * Math.Sin(2.0 * Math.PI * u2);
        return mean + (stddev * randStdNormal);
    }
}

internal static class DeviceMeasurementResultExtensions
{
    public static DeviceMeasurementResult WithPoints(this DeviceMeasurementResult result, IEnumerable<MeasurementPoint> points)
    {
        foreach (var point in points)
        {
            result.Points.Add(point);
        }

        return result;
    }
}
