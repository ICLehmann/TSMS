using System.Diagnostics;
using System.Net.Sockets;
using System.Text;
using TSMS.Meter.Abstractions;

namespace TSMS.Meter.Tcp;

public sealed class TcpMeterDeviceModule : IMeterDeviceModule
{
    private MeterModuleSettings _settings = new();

    public string TransportName => "TCP";

    public Task InitializeAsync(MeterModuleSettings settings, CancellationToken cancellationToken)
    {
        _settings = settings;
        return Task.CompletedTask;
    }

    public Task StopAsync(CancellationToken cancellationToken)
    {
        return Task.CompletedTask;
    }

    public async Task<DeviceMeasurementResult> MeasureDeviceAsync(
        MeterDeviceRequest request,
        DeviceTextConfig? deviceConfig,
        CancellationToken cancellationToken)
    {
        var stopwatch = Stopwatch.StartNew();

        try
        {
            // Device-specific endpoint settings intentionally mirror legacy config style.
            var host = ResolveHost(request, deviceConfig);
            var port = ResolvePort(request, deviceConfig);
            if (port <= 0)
            {
                throw new InvalidOperationException($"Missing valid TCP port for device '{request.DeviceId}'.");
            }

            var connectTimeoutMs = ReadInt("Tcp:ConnectTimeoutMs", 2000);
            var readTimeoutMs = ReadInt("Tcp:ReadTimeoutMs", 2000);
            var lineEnding = ReadSetting("Tcp:LineEnding", "\r\n");
            var startCommands = ResolveStartCommands(request, deviceConfig);
            var readCommands = ResolveReadCommands(request, deviceConfig);
            var waitAfterStartMs = ResolveWaitAfterStart(deviceConfig);

            using var client = new TcpClient();
            using var connectCts = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
            connectCts.CancelAfter(connectTimeoutMs);
            await client.ConnectAsync(host, port, connectCts.Token);

            using var stream = client.GetStream();
            stream.ReadTimeout = readTimeoutMs;
            stream.WriteTimeout = readTimeoutMs;

            string? rawResponse = null;
            foreach (var startCommand in startCommands)
            {
                var response = await SendAndMaybeReadAsync(stream, startCommand + lineEnding, cancellationToken);
                if (!string.IsNullOrWhiteSpace(response))
                {
                    rawResponse = response;
                }
            }

            if (waitAfterStartMs > 0)
            {
                await Task.Delay(waitAfterStartMs, cancellationToken);
            }

            foreach (var readCommand in readCommands)
            {
                var response = await SendAndMaybeReadAsync(stream, readCommand + lineEnding, cancellationToken, forceRead: true);
                if (!string.IsNullOrWhiteSpace(response))
                {
                    rawResponse = response;
                }
            }

            if (string.IsNullOrWhiteSpace(rawResponse))
            {
                rawResponse = await ReadLineAsync(stream, cancellationToken);
            }

            var values = MeterValueEvaluation.ParseCsvValues(rawResponse);

            var result = new DeviceMeasurementResult
            {
                DeviceId = request.DeviceId,
                DeviceType = request.DeviceType,
                DurationMs = (int)stopwatch.ElapsedMilliseconds
            };

            for (var i = 0; i < request.Channels.Count; i++)
            {
                var channel = request.Channels[i];
                // Respect configured telegram positions if present; otherwise fallback to channel index.
                var valueIndex = ResolveValueIndex(channel.Name, i, deviceConfig);
                var value = valueIndex < values.Count ? values[valueIndex] : 0.0;
                result.Points.Add(new MeasurementPoint
                {
                    DeviceId = request.DeviceId,
                    DeviceType = request.DeviceType,
                    Name = channel.Name,
                    Value = value,
                    Evaluation = MeterValueEvaluation.Evaluate(value, channel),
                    StationNumber = request.StationNumber,
                    TimeMs = (int)stopwatch.ElapsedMilliseconds,
                    PatEnabled = channel.PatEnabled,
                    PatMin = channel.PatMin,
                    PatMax = channel.PatMax
                });
            }

            return result;
        }
        catch (Exception ex) when (ex is not OperationCanceledException)
        {
            return MeterValueEvaluation.CreateFailureResult(
                request,
                $"TCP measurement failed: {ex.Message}",
                (int)stopwatch.ElapsedMilliseconds);
        }
    }

    private async Task SendCommandAsync(NetworkStream stream, string command, CancellationToken cancellationToken)
    {
        var bytes = Encoding.ASCII.GetBytes(command);
        await stream.WriteAsync(bytes, 0, bytes.Length, cancellationToken);
    }

    private async Task<string?> SendAndMaybeReadAsync(
        NetworkStream stream,
        string command,
        CancellationToken cancellationToken,
        bool forceRead = false)
    {
        await SendCommandAsync(stream, command, cancellationToken);
        if (forceRead || command.Contains('?', StringComparison.Ordinal))
        {
            return await ReadLineAsync(stream, cancellationToken);
        }

        return null;
    }

    private static async Task<string> ReadLineAsync(NetworkStream stream, CancellationToken cancellationToken)
    {
        var buffer = new byte[1024];
        var sb = new StringBuilder();
        while (true)
        {
            var read = await stream.ReadAsync(buffer, 0, buffer.Length, cancellationToken);
            if (read <= 0)
            {
                break;
            }

            sb.Append(Encoding.ASCII.GetString(buffer, 0, read));
            if (sb.ToString().Contains('\n'))
            {
                break;
            }
        }

        return sb.ToString().Trim();
    }

    private int ReadInt(string key, int fallback)
    {
        if (_settings.Parameters.TryGetValue(key, out var value) && int.TryParse(value, out var parsed))
        {
            return parsed;
        }

        return fallback;
    }

    private string ReadRequired(string key)
    {
        if (_settings.Parameters.TryGetValue(key, out var value) && !string.IsNullOrWhiteSpace(value))
        {
            return value;
        }

        throw new InvalidOperationException($"Missing setting '{key}'.");
    }

    private string ReadSetting(string key, string fallback)
    {
        if (_settings.Parameters.TryGetValue(key, out var value) && !string.IsNullOrWhiteSpace(value))
        {
            return value;
        }

        return fallback;
    }

    private string ResolveHost(MeterDeviceRequest request, DeviceTextConfig? deviceConfig)
    {
        if (deviceConfig is not null && !string.IsNullOrWhiteSpace(deviceConfig.Address))
        {
            return deviceConfig.Address;
        }

        return ReadRequired($"Tcp:{request.DeviceId}:Host");
    }

    private int ResolvePort(MeterDeviceRequest request, DeviceTextConfig? deviceConfig)
    {
        if (deviceConfig is not null && deviceConfig.PortOrDevice > 0)
        {
            return deviceConfig.PortOrDevice;
        }

        return ReadInt($"Tcp:{request.DeviceId}:Port", 0);
    }

    private List<string> ResolveStartCommands(MeterDeviceRequest request, DeviceTextConfig? deviceConfig)
    {
        if (deviceConfig is not null && deviceConfig.StartMeasurementSequence.Count > 0)
        {
            return deviceConfig.StartMeasurementSequence;
        }

        var startCommand = ReadSetting($"Tcp:{request.DeviceId}:StartCommand", string.Empty);
        return string.IsNullOrWhiteSpace(startCommand) ? new List<string>() : new List<string> { startCommand };
    }

    private List<string> ResolveReadCommands(MeterDeviceRequest request, DeviceTextConfig? deviceConfig)
    {
        if (deviceConfig is not null && deviceConfig.GetAnswerSequence.Count > 0)
        {
            return deviceConfig.GetAnswerSequence;
        }

        var readCommand = ReadSetting($"Tcp:{request.DeviceId}:ReadCommand", "READ?");
        return string.IsNullOrWhiteSpace(readCommand) ? new List<string>() : new List<string> { readCommand };
    }

    private static int ResolveWaitAfterStart(DeviceTextConfig? deviceConfig)
    {
        return deviceConfig?.WaitAfterStartMs ?? 0;
    }

    private static int ResolveValueIndex(string channelName, int fallbackIndex, DeviceTextConfig? deviceConfig)
    {
        if (deviceConfig is null)
        {
            return fallbackIndex;
        }

        var valueDef = deviceConfig.Values.FirstOrDefault(v =>
            string.Equals(v.Name, channelName, StringComparison.OrdinalIgnoreCase));
        return valueDef is null ? fallbackIndex : valueDef.Position;
    }
}
