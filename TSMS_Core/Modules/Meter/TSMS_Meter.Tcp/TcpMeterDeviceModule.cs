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

    public async Task<DeviceMeasurementResult> MeasureDeviceAsync(MeterDeviceRequest request, CancellationToken cancellationToken)
    {
        var stopwatch = Stopwatch.StartNew();

        try
        {
            // Device-specific endpoint settings intentionally mirror legacy config style.
            var host = ReadRequired($"Tcp:{request.DeviceId}:Host");
            var port = ReadInt($"Tcp:{request.DeviceId}:Port", 0);
            if (port <= 0)
            {
                throw new InvalidOperationException($"Missing valid TCP port for device '{request.DeviceId}'.");
            }

            var connectTimeoutMs = ReadInt("Tcp:ConnectTimeoutMs", 2000);
            var readTimeoutMs = ReadInt("Tcp:ReadTimeoutMs", 2000);
            var lineEnding = ReadSetting("Tcp:LineEnding", "\r\n");
            var startCommand = ReadSetting($"Tcp:{request.DeviceId}:StartCommand", string.Empty);
            var readCommand = ReadSetting($"Tcp:{request.DeviceId}:ReadCommand", "READ?");

            using var client = new TcpClient();
            using var connectCts = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
            connectCts.CancelAfter(connectTimeoutMs);
            await client.ConnectAsync(host, port, connectCts.Token);

            using var stream = client.GetStream();
            stream.ReadTimeout = readTimeoutMs;
            stream.WriteTimeout = readTimeoutMs;

            if (!string.IsNullOrWhiteSpace(startCommand))
            {
                await SendCommandAsync(stream, startCommand + lineEnding, cancellationToken);
            }

            await SendCommandAsync(stream, readCommand + lineEnding, cancellationToken);
            var rawResponse = await ReadLineAsync(stream, cancellationToken);
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
                // If fewer values are returned than expected, missing values are treated as 0.0.
                var value = i < values.Count ? values[i] : 0.0;
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
}
