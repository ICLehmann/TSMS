using System.Diagnostics;
using TSMS.Meter.Abstractions;

namespace TSMS.Meter.Gpib;

public sealed class GpibMeterDeviceModule : IMeterDeviceModule
{
    private readonly Random _random = new();
    private readonly object _sync = new();
    private MeterModuleSettings _settings = new();

    public string TransportName => "GPIB";

    public Task InitializeAsync(MeterModuleSettings settings, CancellationToken cancellationToken)
    {
        _settings = settings;
        return Task.CompletedTask;
    }

    public Task StopAsync(CancellationToken cancellationToken)
    {
        return Task.CompletedTask;
    }

    public Task<DeviceMeasurementResult> MeasureDeviceAsync(MeterDeviceRequest request, CancellationToken cancellationToken)
    {
        var stopwatch = Stopwatch.StartNew();
        var allowFallback = ReadBool("Gpib:EnableSimulationFallback", true);
        if (!allowFallback)
        {
            throw new NotSupportedException("GPIB module needs a platform adapter. Enable Gpib:EnableSimulationFallback to simulate.");
        }

        // Current implementation keeps the contract alive via simulation fallback.
        var result = new DeviceMeasurementResult
        {
            DeviceId = request.DeviceId,
            DeviceType = request.DeviceType,
            DurationMs = 0
        };

        foreach (var channel in request.Channels)
        {
            var simulated = CreateValue(channel);
            result.Points.Add(new MeasurementPoint
            {
                DeviceId = request.DeviceId,
                DeviceType = request.DeviceType,
                Name = channel.Name,
                Value = simulated,
                Evaluation = MeterValueEvaluation.Evaluate(simulated, channel),
                StationNumber = request.StationNumber,
                TimeMs = (int)stopwatch.ElapsedMilliseconds,
                PatEnabled = channel.PatEnabled,
                PatMin = channel.PatMin,
                PatMax = channel.PatMax
            });
        }

        result = new DeviceMeasurementResult
        {
            DeviceId = result.DeviceId,
            DeviceType = result.DeviceType,
            DurationMs = (int)stopwatch.ElapsedMilliseconds,
            ErrorMessage = "Simulated fallback (no hardware adapter configured)"
        }.WithPoints(result.Points);

        return Task.FromResult(result);
    }

    private bool ReadBool(string key, bool fallback)
    {
        if (_settings.Parameters.TryGetValue(key, out var value) && bool.TryParse(value, out var parsed))
        {
            return parsed;
        }

        return fallback;
    }

    private double CreateValue(MeterChannelRequest channel)
    {
        var mean = string.Equals(channel.Name, "ISO", StringComparison.OrdinalIgnoreCase)
            ? channel.Min + ((channel.Max - channel.Min) / 2.0)
            : channel.Nominal;

        var stddev = Math.Abs(channel.Max - channel.Min) / 100.0 * 5.0;
        if (stddev <= 0)
        {
            stddev = 0.001;
        }

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
