using System.Net;
using System.Net.Sockets;
using TSMS.Machine.Abstractions;

namespace TSMS.Core.GuiUdp;

public sealed class GuiUdpPublisher : IAsyncDisposable
{
    private static readonly string[] MeasurementOrder = { "ISO", "L", "Z1", "Z2", "Z3", "C1", "C2", "C3", "R1", "R2" };
    private readonly GuiUdpOutputOptions _options;
    private readonly UdpClient _sender;
    private readonly IPEndPoint _sendEndpoint;

    public GuiUdpPublisher(GuiUdpOutputOptions options)
    {
        _options = options;
        _sender = new UdpClient();
        _sendEndpoint = new IPEndPoint(IPAddress.Parse(options.IpAddress), options.SendPort);
    }

    public async Task PublishCoreSnapshotAsync(
        CoreResult result,
        bool lotInProgress,
        GuiLotData? lotData,
        GuiCounterSnapshot? counter,
        IReadOnlyList<GuiMeasuredValue>? measuredValues,
        CancellationToken cancellationToken)
    {
        await SendAsync(GuiUdpTelegramBuilder.BuildSetup(_options.MachineName), cancellationToken);
        await SendAsync(GuiUdpTelegramBuilder.BuildStatus(result), cancellationToken);
        await SendAsync(GuiUdpTelegramBuilder.BuildMachineSignals(lotInProgress), cancellationToken);

        if (lotData is not null)
        {
            await SendAsync(GuiUdpTelegramBuilder.BuildLotData(lotData), cancellationToken);
        }

        if (counter is not null)
        {
            await SendAsync(GuiUdpTelegramBuilder.BuildCounter(counter), cancellationToken);
        }

        if (measuredValues is not null)
        {
            foreach (var value in measuredValues)
            {
                await SendAsync(GuiUdpTelegramBuilder.BuildMeasuredValue(value), cancellationToken);
            }
        }

        var popup = !result.Success || result.Status.Contains("Rejected", StringComparison.OrdinalIgnoreCase);
        await SendAsync(GuiUdpTelegramBuilder.BuildUserMessage(result.Status, popup), cancellationToken);
    }

    public static GuiLotData CreateLotData(ulong lotNumber)
    {
        return new GuiLotData
        {
            LotNumber = lotNumber,
            ProductNumber = "HU-SIM",
            LineId = "SIM-LINE",
            OperatorId = "SIM",
            Configs = BuildDefaultValueConfigs()
        };
    }

    public static GuiCounterSnapshot CreateEmptyCounter()
    {
        return new GuiCounterSnapshot
        {
            Pass = new uint[MeasurementOrder.Length],
            Low = new uint[MeasurementOrder.Length],
            High = new uint[MeasurementOrder.Length],
            Fail = new uint[MeasurementOrder.Length]
        };
    }

    public static IReadOnlyList<GuiMeasuredValue> ExtractMeasuredValues(CoreResult result)
    {
        var measurements = new List<GuiMeasuredValue>();
        for (ushort i = 0; i < MeasurementOrder.Length; i++)
        {
            var name = MeasurementOrder[i];
            if (!result.Data.TryGetValue($"Value:{name}", out var valueText) ||
                !double.TryParse(valueText, System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out var value))
            {
                continue;
            }

            measurements.Add(new GuiMeasuredValue
            {
                Type = i,
                Value = value,
                PatEnabled = false
            });
        }

        return measurements;
    }

    public static void ApplyMeasurementToCounter(GuiCounterSnapshot counter, CoreResult result)
    {
        counter.Total++;
        if (result.Success)
        {
            counter.Good++;
        }

        for (var i = 0; i < MeasurementOrder.Length; i++)
        {
            if (!result.Data.TryGetValue($"Result:{MeasurementOrder[i]}", out var eval) || string.IsNullOrWhiteSpace(eval))
            {
                continue;
            }

            switch (eval.Trim().ToUpperInvariant())
            {
                case "PASS":
                    counter.Pass[i]++;
                    break;
                case "LOW":
                    counter.Low[i]++;
                    break;
                case "HIGH":
                    counter.High[i]++;
                    break;
                case "FAIL":
                    counter.Fail[i]++;
                    break;
            }
        }
    }

    public ValueTask DisposeAsync()
    {
        _sender.Dispose();
        return ValueTask.CompletedTask;
    }

    private async Task SendAsync(byte[] payload, CancellationToken cancellationToken)
    {
        if (payload.Length == 0)
        {
            return;
        }

        await _sender.SendAsync(payload, _sendEndpoint, cancellationToken);
    }

    private static List<GuiValueConfig> BuildDefaultValueConfigs()
    {
        return
        [
            new GuiValueConfig { Min = 500.0, Nom = 800.0, Max = 1500.0, Type = 0 },
            new GuiValueConfig { Min = 0.8, Nom = 1.0, Max = 1.2, Type = 1 },
            new GuiValueConfig { Min = 8.0, Nom = 10.0, Max = 12.0, Type = 2 },
            new GuiValueConfig { Min = 8.0, Nom = 10.0, Max = 12.0, Type = 3 },
            new GuiValueConfig { Min = 8.0, Nom = 10.0, Max = 12.0, Type = 4 },
            new GuiValueConfig { Min = 80.0, Nom = 100.0, Max = 120.0, Type = 5 },
            new GuiValueConfig { Min = 80.0, Nom = 100.0, Max = 120.0, Type = 6 },
            new GuiValueConfig { Min = 80.0, Nom = 100.0, Max = 120.0, Type = 7 },
            new GuiValueConfig { Min = 8.0, Nom = 10.0, Max = 12.0, Type = 8 },
            new GuiValueConfig { Min = 8.0, Nom = 10.0, Max = 12.0, Type = 9 }
        ];
    }
}

public sealed class GuiLotData
{
    public ulong LotNumber { get; init; }
    public string ProductNumber { get; init; } = string.Empty;
    public string LineId { get; init; } = string.Empty;
    public string OperatorId { get; init; } = string.Empty;
    public IReadOnlyList<GuiValueConfig> Configs { get; init; } = Array.Empty<GuiValueConfig>();
}

public sealed class GuiValueConfig
{
    public double Min { get; init; }
    public double Nom { get; init; }
    public double Max { get; init; }
    public double Offset { get; init; }
    public char Prefix { get; init; } = ' ';
    public double ErrorRate { get; init; } = 0.02;
    public ulong Frequency { get; init; } = 1_000_000;
    public double StimuliLevel { get; init; } = 0.1;
    public ushort Type { get; init; }
    public bool PatEnabled { get; init; }
    public double PatMin { get; init; }
    public double PatMax { get; init; }
}

public sealed class GuiCounterSnapshot
{
    public uint Total { get; set; }
    public uint Good { get; set; }
    public uint[] Pass { get; init; } = Array.Empty<uint>();
    public uint[] Low { get; init; } = Array.Empty<uint>();
    public uint[] High { get; init; } = Array.Empty<uint>();
    public uint[] Fail { get; init; } = Array.Empty<uint>();
}

public sealed class GuiMeasuredValue
{
    public ushort Type { get; init; }
    public double Value { get; init; }
    public bool PatEnabled { get; init; }
    public double PatMin { get; init; }
    public double PatMax { get; init; }
}
