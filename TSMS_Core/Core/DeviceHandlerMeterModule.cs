using System.Collections.Concurrent;
using System.Diagnostics;
using TSMS.Meter.Abstractions;
using TSMS.Meter.Gpib;
using TSMS.Meter.Sim;
using TSMS.Meter.Tcp;

namespace TSMS.Core;

public sealed class DeviceHandlerMeterModule : IMeterModule
{
    // Cache one module instance per transport to reuse connections and state.
    private readonly ConcurrentDictionary<string, IMeterDeviceModule> _transportModules = new(StringComparer.OrdinalIgnoreCase);
    private MeterModuleSettings _settings = new();
    private int _timeoutMs;
    private bool _initialized;

    public Task InitializeAsync(MeterModuleSettings settings, CancellationToken cancellationToken)
    {
        _settings = settings;
        _timeoutMs = ReadInt("DeviceHandler:TimeoutMs", 5000);
        _initialized = true;
        return Task.CompletedTask;
    }

    public async Task<MeasurementResponse> MeasureAsync(MeasurementRequest request, CancellationToken cancellationToken)
    {
        if (!_initialized)
        {
            throw new InvalidOperationException("DeviceHandlerMeterModule is not initialized.");
        }

        var response = new MeasurementResponse
        {
            CorrelationId = request.CorrelationId
        };

        if (request.Devices.Count == 0)
        {
            return response;
        }

        using var linkedCts = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        linkedCts.CancelAfter(_timeoutMs);

        // Mirror legacy DeviceHandler behavior: run enabled devices in parallel.
        var tasks = request.Devices
            .Where(d => d.Channels.Count > 0)
            .Select(d => MeasureSafelyAsync(d, linkedCts.Token))
            .ToArray();

        var results = await Task.WhenAll(tasks);
        foreach (var deviceResult in results)
        {
            response.Devices.Add(deviceResult);
            response.Points.AddRange(deviceResult.Points);
        }

        return response;
    }

    public async Task StopAsync(CancellationToken cancellationToken)
    {
        foreach (var module in _transportModules.Values)
        {
            await module.StopAsync(cancellationToken);
        }

        _transportModules.Clear();
        _initialized = false;
    }

    private async Task<DeviceMeasurementResult> MeasureSafelyAsync(MeterDeviceRequest request, CancellationToken cancellationToken)
    {
        var stopwatch = Stopwatch.StartNew();
        try
        {
            var transportName = ResolveTransportName(request);
            // Dispatch to the transport-specific implementation (SIM/TCP/GPIB).
            var module = await GetOrCreateTransportModuleAsync(transportName, cancellationToken);
            return await module.MeasureDeviceAsync(request, cancellationToken);
        }
        catch (OperationCanceledException)
        {
            return MeterValueEvaluation.CreateFailureResult(
                request,
                $"Measurement timed out after {_timeoutMs} ms.",
                (int)stopwatch.ElapsedMilliseconds);
        }
        catch (Exception ex)
        {
            return MeterValueEvaluation.CreateFailureResult(
                request,
                ex.Message,
                (int)stopwatch.ElapsedMilliseconds);
        }
    }

    private async Task<IMeterDeviceModule> GetOrCreateTransportModuleAsync(string transportName, CancellationToken cancellationToken)
    {
        if (_transportModules.TryGetValue(transportName, out var existing))
        {
            return existing;
        }

        var module = CreateTransportModule(transportName);
        await module.InitializeAsync(_settings, cancellationToken);
        _transportModules.TryAdd(transportName, module);
        return _transportModules[transportName];
    }

    private IMeterDeviceModule CreateTransportModule(string transportName)
    {
        return transportName.ToUpperInvariant() switch
        {
            "SIM" => new SimMeterDeviceModule(),
            "TCP" => new TcpMeterDeviceModule(),
            "GPIB" => new GpibMeterDeviceModule(),
            _ => throw new InvalidOperationException($"Unsupported meter transport '{transportName}'.")
        };
    }

    private string ResolveTransportName(MeterDeviceRequest request)
    {
        // Resolution order: per-device override -> per-type default -> global default.
        var byDevice = ReadString($"DeviceHandler:Transport:Device:{request.DeviceId}");
        if (!string.IsNullOrWhiteSpace(byDevice))
        {
            return byDevice;
        }

        var byType = ReadString($"DeviceHandler:Transport:Type:{request.DeviceType}");
        if (!string.IsNullOrWhiteSpace(byType))
        {
            return byType;
        }

        var defaultTransport = ReadString("DeviceHandler:Transport:Default");
        return string.IsNullOrWhiteSpace(defaultTransport) ? "SIM" : defaultTransport;
    }

    private int ReadInt(string key, int defaultValue)
    {
        if (_settings.Parameters.TryGetValue(key, out var value) && int.TryParse(value, out var parsed))
        {
            return parsed;
        }

        return defaultValue;
    }

    private string? ReadString(string key)
    {
        if (_settings.Parameters.TryGetValue(key, out var value) && !string.IsNullOrWhiteSpace(value))
        {
            return value;
        }

        return null;
    }
}
