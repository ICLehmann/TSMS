using TSMS.Machine.Abstractions;

namespace TSMS.Machine.Sim;

public sealed class SimMachineModule : IMachineModule
{
    private MachineModuleSettings _settings = new();
    private CancellationTokenSource? _scenarioCts;
    private Task? _scenarioTask;
    private bool _measureC;
    private int _partsPerLot;
    private int _clockMs;
    private string _mode = "Lot";
    private bool _lotInProgress;
    private bool _lotInProgressConfirmedSent;

    public event EventHandler<MachineTrigger>? TriggerReceived;

    public Task InitializeAsync(MachineModuleSettings settings, CancellationToken cancellationToken)
    {
        _settings = settings;
        _partsPerLot = ReadInt("PartsPerLot", 100);
        _clockMs = ReadInt("ClockMs", 300);
        _measureC = ReadBool("MeasureC", false);
        _mode = ReadString("SimulationMode", "Lot");
        Console.WriteLine($"[SimModule] Initialized for machine: {_settings.MachineId}");
        return Task.CompletedTask;
    }

    public Task StartAsync(CancellationToken cancellationToken)
    {
        _scenarioCts = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        _scenarioTask = Task.Run(() => RunScenarioAsync(_scenarioCts.Token), _scenarioCts.Token);
        return Task.CompletedTask;
    }

    public Task PublishResultAsync(CoreResult result, CancellationToken cancellationToken)
    {
        Console.WriteLine($"[SimModule] Result: {result.Status} | Success={result.Success} | Correlation={result.CorrelationId}");
        foreach (var entry in result.Data
                     .Where(x => x.Key.StartsWith("Value:", StringComparison.OrdinalIgnoreCase))
                     .OrderBy(x => x.Key, StringComparer.OrdinalIgnoreCase))
        {
            Console.WriteLine($"[SimModule] {entry.Key[6..]} = {entry.Value}");
        }

        foreach (var entry in result.Data
                     .Where(x => x.Key.StartsWith("Device:", StringComparison.OrdinalIgnoreCase) && x.Key.EndsWith(":DurationMs", StringComparison.OrdinalIgnoreCase))
                     .OrderBy(x => x.Key, StringComparer.OrdinalIgnoreCase))
        {
            var deviceId = entry.Key[7..^11];
            Console.WriteLine($"[SimModule] Device {deviceId} finished in {entry.Value} ms");
        }

        if (result.Data.TryGetValue("Command", out var command) &&
            result.Data.TryGetValue("Value", out var value) &&
            string.Equals(command, "SetLotInProgress", StringComparison.OrdinalIgnoreCase))
        {
            _lotInProgress = string.Equals(value, "1", StringComparison.OrdinalIgnoreCase);
            if (_lotInProgress && !_lotInProgressConfirmedSent)
            {
                _lotInProgressConfirmedSent = true;
                RaiseTrigger(new MachineTrigger(
                    Guid.NewGuid().ToString("N"),
                    TriggerKind.LotInProgressConfirmed,
                    lotId: _settings.MachineId,
                    data: new Dictionary<string, string?> { ["MachineReady"] = "1" }));
            }
        }

        return Task.CompletedTask;
    }

    public async Task StopAsync(CancellationToken cancellationToken)
    {
        if (_scenarioCts is not null)
        {
            _scenarioCts.Cancel();
        }

        if (_scenarioTask is not null)
        {
            try
            {
                await _scenarioTask;
            }
            catch (OperationCanceledException)
            {
            }
        }

        Console.WriteLine("[SimModule] Stopped.");
    }

    private void RaiseTrigger(MachineTrigger trigger)
    {
        TriggerReceived?.Invoke(this, trigger);
    }

    private async Task RunScenarioAsync(CancellationToken cancellationToken)
    {
        Console.WriteLine($"[SimModule] Starting simulation: Mode={_mode}, Parts={_partsPerLot}, ClockMs={_clockMs}, MeasureC={_measureC}");

        RaiseTrigger(new MachineTrigger(Guid.NewGuid().ToString("N"), TriggerKind.StartupRequested));
        await Task.Delay(250, cancellationToken);

        if (string.Equals(_mode, "Dummy", StringComparison.OrdinalIgnoreCase))
        {
            await RunDummyScenarioAsync(cancellationToken);
        }
        else if (string.Equals(_mode, "Compensation", StringComparison.OrdinalIgnoreCase))
        {
            await RunCompensationScenarioAsync(cancellationToken);
        }
        else
        {
            await RunLotScenarioAsync(cancellationToken);
        }

        RaiseTrigger(new MachineTrigger(Guid.NewGuid().ToString("N"), TriggerKind.ShutdownRequested));
    }

    private async Task RunLotScenarioAsync(CancellationToken cancellationToken)
    {
        _lotInProgress = false;
        _lotInProgressConfirmedSent = false;
        var lotId = $"SIM-LOT-{DateTime.Now:HHmmss}";

        RaiseTrigger(new MachineTrigger(Guid.NewGuid().ToString("N"), TriggerKind.LotStartRequested, lotId));
        await WaitUntilAsync(() => _lotInProgress, TimeSpan.FromSeconds(10), cancellationToken);
        await Task.Delay(250, cancellationToken);

        var stations = InitializeStations();
        await RunOldStyleLotFlowAsync(stations, lotId, cancellationToken);

        RaiseTrigger(new MachineTrigger(Guid.NewGuid().ToString("N"), TriggerKind.LotEndRequested, lotId));
        await Task.Delay(500, cancellationToken);
    }

    private async Task RunDummyScenarioAsync(CancellationToken cancellationToken)
    {
        var lotId = $"SIM-DUMMY-{DateTime.Now:HHmmss}";
        RaiseTrigger(new MachineTrigger(Guid.NewGuid().ToString("N"), TriggerKind.DummyStartRequested, lotId));
        await Task.Delay(300, cancellationToken);

        var stations = InitializeStations();
        await RunOldStyleDummyFlowAsync(stations, lotId, cancellationToken);

        RaiseTrigger(new MachineTrigger(Guid.NewGuid().ToString("N"), TriggerKind.DummyStopRequested, lotId));
    }

    private async Task RunCompensationScenarioAsync(CancellationToken cancellationToken)
    {
        var lotId = $"SIM-COMP-{DateTime.Now:HHmmss}";
        RaiseTrigger(new MachineTrigger(Guid.NewGuid().ToString("N"), TriggerKind.CompensationStartRequested, lotId));
        await Task.Delay(1500, cancellationToken);
        RaiseTrigger(new MachineTrigger(Guid.NewGuid().ToString("N"), TriggerKind.CompensationStopRequested, lotId));
    }

    private async Task RunOldStyleLotFlowAsync(List<string> stations, string lotId, CancellationToken cancellationToken)
    {
        var phaseIndex = 0;
        var phases = new List<Dictionary<string, string?>>
        {
            BuildCycleFromStations(stations, true, true, true, true),
            BuildCycleFromStations(stations, true, true, true, true),
            BuildCycleFromStations(stations, true, true, true, true),
            BuildCycleFromStations(stations, true, true, true, true),
            BuildCycleFromStations(stations, true, true, true, true),
        };

        foreach (var phase in phases)
        {
            phase["Pattern"] = $"StartupPhase-{phaseIndex++}";
            await SendMeasurementCycleAsync(lotId, phase, cancellationToken, initialDelayMs: 500, settleDelayMs: _clockMs);
        }

        var repeated = BuildCycleFromStations(stations, true, true, true, true);
        for (var i = 0; i < Math.Max(1, _partsPerLot); i++)
        {
            repeated["Pattern"] = $"Cycle-{i + 1}";
            await SendMeasurementCycleAsync(lotId, repeated, cancellationToken, initialDelayMs: 250, settleDelayMs: _clockMs);
        }
    }

    private async Task RunOldStyleDummyFlowAsync(List<string> stations, string lotId, CancellationToken cancellationToken)
    {
        var phases = new List<Dictionary<string, string?>>
        {
            BuildCycleFromStations(stations, true, true, true, true),
            BuildCycleFromStations(stations, true, true, true, true),
            BuildCycleFromStations(stations, true, true, true, true),
            BuildCycleFromStations(stations, true, true, true, true),
        };

        var phaseIndex = 0;
        foreach (var phase in phases)
        {
            phase["Pattern"] = $"DummyPhase-{phaseIndex++}";
            await SendMeasurementCycleAsync(lotId, phase, cancellationToken, initialDelayMs: 600, settleDelayMs: _clockMs);
        }
    }

    private async Task RunStationFlowAsync(Queue<string> parts, List<string> stations, string lotId, CancellationToken cancellationToken)
    {
        const string noTrigger = "000000";

        while (!cancellationToken.IsCancellationRequested)
        {
            stations.RemoveAt(stations.Count - 1);
            if (parts.Count > 0)
            {
                stations.Insert(0, parts.Dequeue());
            }
            else
            {
                if (stations[^1] == noTrigger)
                {
                    break;
                }

                stations.Insert(0, noTrigger);
            }

            var values = BuildCycleValues(stations);
            if (values.Count > 0)
            {
                RaiseTrigger(new MachineTrigger(
                    Guid.NewGuid().ToString("N"),
                    TriggerKind.MeasurementCycleCompleted,
                    lotId,
                    values));
            }

            await Task.Delay(_clockMs + 150, cancellationToken);
        }
    }

    private Dictionary<string, string?> BuildCycleValues(IReadOnlyList<string> stations)
    {
        return BuildCycleFromStations(stations, true, true, _measureC, true);
    }

    private Dictionary<string, string?> BuildCycleFromStations(IReadOnlyList<string> stations, bool iso, bool l, bool c, bool resistances)
    {
        var data = new Dictionary<string, string?>(StringComparer.OrdinalIgnoreCase);

        if (iso)
        {
            data["TriggerIso"] = "1";
        }

        if (l)
        {
            data["TriggerInductanceImpedance"] = "1";
        }

        if (c && _measureC)
        {
            data["TriggerCapacity"] = "1";
        }

        if (resistances)
        {
            data["TriggerResistance"] = "1";
        }

        if (stations[0][0] == '1')
        {
            data["IncludeZ2"] = "1";
        }

        if (stations[1][1] == '1')
        {
            data["IncludeZ3"] = "1";
        }

        return data;
    }

    private async Task SendMeasurementCycleAsync(string lotId, Dictionary<string, string?> values, CancellationToken cancellationToken, int initialDelayMs, int settleDelayMs)
    {
        await Task.Delay(initialDelayMs, cancellationToken);
        RaiseTrigger(new MachineTrigger(
            Guid.NewGuid().ToString("N"),
            TriggerKind.MeasurementCycleCompleted,
            lotId,
            values));
        await Task.Delay(settleDelayMs, cancellationToken);
    }

    private static List<string> InitializeStations()
    {
        return new List<string> { "000000", "000000", "000000", "000000", "000000", "000000" };
    }

    private async Task WaitUntilAsync(Func<bool> condition, TimeSpan timeout, CancellationToken cancellationToken)
    {
        var start = DateTime.UtcNow;
        while (!condition())
        {
            if (DateTime.UtcNow - start > timeout)
            {
                throw new TimeoutException("Simulation timeout: LOT_IN_PROGRESS was not set.");
            }

            await Task.Delay(20, cancellationToken);
        }
    }

    private string ReadString(string key, string defaultValue)
    {
        return _settings.Parameters.TryGetValue(key, out var value) && !string.IsNullOrWhiteSpace(value)
            ? value
            : defaultValue;
    }

    private int ReadInt(string key, int defaultValue)
    {
        if (_settings.Parameters.TryGetValue(key, out var value) && int.TryParse(value, out var parsed))
        {
            return parsed;
        }

        return defaultValue;
    }

    private bool ReadBool(string key, bool defaultValue)
    {
        if (_settings.Parameters.TryGetValue(key, out var value) && bool.TryParse(value, out var parsed))
        {
            return parsed;
        }

        if (_settings.Parameters.TryGetValue(key, out var bit))
        {
            if (bit == "1")
            {
                return true;
            }

            if (bit == "0")
            {
                return false;
            }
        }

        return defaultValue;
    }
}
