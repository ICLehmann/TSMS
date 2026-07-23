using TSMS.Machine.Abstractions;
using TSMS.Meter.Abstractions;
using TSMS.Core.GuiUdp;

namespace TSMS.Core;

public sealed class CoreEngine
{
    private readonly CoreOptions _options;
    private readonly IMachineModule _machineModule;
    private readonly IMeterModule _meterModule;
    private readonly ResultEvaluator _resultEvaluator;
    private GuiUdpPublisher? _guiPublisher;
    private GuiCounterSnapshot? _guiCounter;
    private bool _shutdownRequested;
    private CoreState _state = CoreState.Booting;
    private string? _currentLotId;
    private ulong _currentLotNumber;

    public CoreEngine(CoreOptions options, IMachineModule machineModule, IMeterModule meterModule)
    {
        _options = options;
        _machineModule = machineModule;
        _meterModule = meterModule;
        _resultEvaluator = new ResultEvaluator(options.Profile);
    }

    public async Task RunAsync(CancellationToken cancellationToken)
    {
        // Translate config dictionaries into strongly typed module settings.
        var machineSettings = new MachineModuleSettings
        {
            MachineId = _options.MachineModule.Settings.TryGetValue("MachineId", out var machineId) ? machineId : _options.Profile.Name,
            Parameters = _options.MachineModule.Settings
        };
        var meterSettings = new MeterModuleSettings
        {
            MeterProfile = _options.Profile.Name,
            Parameters = _options.MeterModule.Settings
        };

        _machineModule.TriggerReceived += OnTriggerReceived;
        await _machineModule.InitializeAsync(machineSettings, cancellationToken);
        await _meterModule.InitializeAsync(meterSettings, cancellationToken);
        if (_options.GuiUdp.Enabled)
        {
            // Keep GUI machine name stable even when not explicitly configured.
            var machineName = _options.MachineModule.Settings.TryGetValue("MachineId", out var configuredMachineName)
                ? configuredMachineName
                : _options.Profile.Name;
            _options.GuiUdp.MachineName = string.IsNullOrWhiteSpace(_options.GuiUdp.MachineName)
                ? machineName
                : _options.GuiUdp.MachineName;
            _guiPublisher = new GuiUdpPublisher(_options.GuiUdp);
        }

        TransitionTo(CoreState.ReadyForNewLot);
        await _machineModule.StartAsync(cancellationToken);

        // Core loop only keeps the process alive; business logic is event-driven.
        while (!cancellationToken.IsCancellationRequested && !_shutdownRequested)
        {
            await Task.Delay(100, cancellationToken);
        }

        await _machineModule.StopAsync(CancellationToken.None);
        await _meterModule.StopAsync(CancellationToken.None);
        if (_guiPublisher is not null)
        {
            await _guiPublisher.DisposeAsync();
        }

        _machineModule.TriggerReceived -= OnTriggerReceived;
    }

    private async void OnTriggerReceived(object? sender, MachineTrigger trigger)
    {
        try
        {
            // Convert machine triggers to a single normalized CoreResult payload.
            var response = await HandleTriggerAsync(trigger);

            await _machineModule.PublishResultAsync(response, CancellationToken.None);
            await PublishGuiSnapshotAsync(response);
        }
        catch (Exception ex)
        {
            var errorResult = CoreResult.Fail(trigger.CorrelationId, $"UnhandledException:{ex.Message}");
            await _machineModule.PublishResultAsync(errorResult, CancellationToken.None);
            await PublishGuiSnapshotAsync(errorResult);
        }
    }

    private Task<CoreResult> HandleTriggerAsync(MachineTrigger trigger)
    {
        // Central state-machine dispatch for all machine-originated trigger kinds.
        switch (trigger.Kind)
        {
            case TriggerKind.StartupRequested:
                return Task.FromResult(BuildResult(trigger.CorrelationId, true, "CoreReady"));

            case TriggerKind.LotStartRequested:
                return Task.FromResult(HandleLotStart(trigger));

            case TriggerKind.LotInProgressConfirmed:
                return Task.FromResult(HandleLotInProgressConfirmed(trigger));

            case TriggerKind.MeasurementCycleCompleted:
                return HandleMeasurementAsync(trigger);

            case TriggerKind.LotEndRequested:
            case TriggerKind.LotPauseRequested:
            case TriggerKind.LotDiscardRequested:
                return Task.FromResult(HandleLotTermination(trigger));

            case TriggerKind.DummyStartRequested:
                return Task.FromResult(HandleDummyStart(trigger));

            case TriggerKind.DummyStopRequested:
                return Task.FromResult(HandleDummyStop(trigger));

            case TriggerKind.CompensationStartRequested:
                return Task.FromResult(HandleCompensationStart(trigger));

            case TriggerKind.CompensationStopRequested:
                return Task.FromResult(HandleCompensationStop(trigger));

            case TriggerKind.ShutdownRequested:
                _shutdownRequested = true;
                TransitionTo(CoreState.ShutdownCommand);
                TransitionTo(CoreState.Exit);
                return Task.FromResult(BuildResult(trigger.CorrelationId, true, "ShutdownAccepted"));

            case TriggerKind.OperatorCommand:
                return Task.FromResult(HandleOperatorCommand(trigger));

            default:
                return Task.FromResult(BuildResult(trigger.CorrelationId, false, $"UnsupportedTrigger:{trigger.Kind}"));
        }
    }

    private CoreResult HandleLotStart(MachineTrigger trigger)
    {
        if (_state != CoreState.ReadyForNewLot)
        {
            return BuildResult(trigger.CorrelationId, false, $"LotStartRejected:State={_state}");
        }

        _currentLotId = trigger.LotId;
        _currentLotNumber = ToLotNumber(trigger.LotId);
        _guiCounter = GuiUdpPublisher.CreateEmptyCounter();
        TransitionTo(CoreState.WaitUntilLotInProgress);

        return BuildResult(trigger.CorrelationId, true, "LotStartAccepted", new Dictionary<string, string>
        {
            ["Command"] = "SetLotInProgress",
            ["Value"] = "1"
        });
    }

    private CoreResult HandleLotInProgressConfirmed(MachineTrigger trigger)
    {
        if (_state != CoreState.WaitUntilLotInProgress)
        {
            return BuildResult(trigger.CorrelationId, false, $"LotInProgressIgnored:State={_state}");
        }

        var machineReady = trigger.Data.TryGetValue("MachineReady", out var machineReadyValue) &&
                           string.Equals(machineReadyValue, "1", StringComparison.OrdinalIgnoreCase);
        if (!machineReady)
        {
            return BuildResult(trigger.CorrelationId, false, "LotInProgressWaitingForMachineReady");
        }

        TransitionTo(CoreState.ReadyToMeasure);
        return BuildResult(trigger.CorrelationId, true, "ReadyToMeasure", new Dictionary<string, string>
        {
            ["Command"] = "SetInitPin",
            ["Value"] = "0"
        });
    }

    private async Task<CoreResult> HandleMeasurementAsync(MachineTrigger trigger)
    {
        if (_state != CoreState.ReadyToMeasure && _state != CoreState.MeasurementRunning)
        {
            return BuildResult(trigger.CorrelationId, false, $"MeasurementRejected:State={_state}");
        }

        TransitionTo(CoreState.MeasurementRunning);

        var request = BuildMeasurementRequest(trigger);
        var meterResponse = await _meterModule.MeasureAsync(request, CancellationToken.None);
        var batch = MeasurementBatch.FromMeterResponse(meterResponse);
        var evaluation = _resultEvaluator.Evaluate(batch);

        TransitionTo(CoreState.MeasurementEvaluate);
        TransitionTo(CoreState.ReadyToMeasure);

        var details = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
        {
            ["PartOk"] = evaluation.PartOk ? "1" : "0",
            ["Rule"] = evaluation.Rule,
            ["LotId"] = _currentLotId ?? string.Empty
        };

        foreach (var item in batch.Values)
        {
            if (!string.IsNullOrWhiteSpace(item.Value))
            {
                // Prefix keys so downstream consumers can parse values by category.
                details[$"Result:{item.Key}"] = item.Value!;
            }
        }

        foreach (var item in batch.NumericValues)
        {
            details[$"Value:{item.Key}"] = item.Value.ToString("F3", System.Globalization.CultureInfo.InvariantCulture);
        }

        foreach (var device in meterResponse.Devices)
        {
            details[$"Device:{device.DeviceId}:DurationMs"] = device.DurationMs.ToString(System.Globalization.CultureInfo.InvariantCulture);
            details[$"Device:{device.DeviceId}:Count"] = device.Points.Count.ToString(System.Globalization.CultureInfo.InvariantCulture);
        }

        if (_guiCounter is not null)
        {
            GuiUdpPublisher.ApplyMeasurementToCounter(_guiCounter, new CoreResult(trigger.CorrelationId, evaluation.PartOk, "MeasurementCompleted", details));
        }

        return new CoreResult(trigger.CorrelationId, evaluation.PartOk, "MeasurementCompleted", details);
    }

    private CoreResult HandleLotTermination(MachineTrigger trigger)
    {
        if (_state != CoreState.ReadyToMeasure && _state != CoreState.MeasurementRunning)
        {
            return BuildResult(trigger.CorrelationId, false, $"LotTerminationRejected:State={_state}");
        }

        TransitionTo(CoreState.ReadyForNewLot);
        _currentLotId = null;
        _currentLotNumber = 0;
        _guiCounter = null;

        return BuildResult(trigger.CorrelationId, true, $"{trigger.Kind}Accepted", new Dictionary<string, string>
        {
            ["Command"] = "SetLotInProgress",
            ["Value"] = "0"
        });
    }

    private CoreResult HandleDummyStart(MachineTrigger trigger)
    {
        if (_state != CoreState.ReadyForNewLot)
        {
            return BuildResult(trigger.CorrelationId, false, $"DummyStartRejected:State={_state}");
        }

        _currentLotId = trigger.LotId;
        TransitionTo(CoreState.ReadyToMeasure);
        return BuildResult(trigger.CorrelationId, true, "DummyStartAccepted");
    }

    private CoreResult HandleDummyStop(MachineTrigger trigger)
    {
        if (_state != CoreState.ReadyToMeasure && _state != CoreState.MeasurementRunning)
        {
            return BuildResult(trigger.CorrelationId, false, $"DummyStopRejected:State={_state}");
        }

        TransitionTo(CoreState.ReadyForNewLot);
        _currentLotId = null;
        return BuildResult(trigger.CorrelationId, true, "DummyStopAccepted");
    }

    private CoreResult HandleCompensationStart(MachineTrigger trigger)
    {
        if (_state != CoreState.ReadyForNewLot)
        {
            return BuildResult(trigger.CorrelationId, false, $"CompensationStartRejected:State={_state}");
        }

        _currentLotId = trigger.LotId;
        TransitionTo(CoreState.Compensation);
        return BuildResult(trigger.CorrelationId, true, "CompensationStartAccepted");
    }

    private CoreResult HandleCompensationStop(MachineTrigger trigger)
    {
        if (_state != CoreState.Compensation)
        {
            return BuildResult(trigger.CorrelationId, false, $"CompensationStopRejected:State={_state}");
        }

        TransitionTo(CoreState.ReadyForNewLot);
        _currentLotId = null;
        return BuildResult(trigger.CorrelationId, true, "CompensationStopAccepted");
    }

    private CoreResult HandleOperatorCommand(MachineTrigger trigger)
    {
        var operation = trigger.Data.TryGetValue("Operation", out var value) ? value : "Unknown";
        return BuildResult(trigger.CorrelationId, true, $"OperatorCommandAccepted:{operation}");
    }

    private void TransitionTo(CoreState nextState)
    {
        _state = nextState;
        Console.WriteLine($"[Core] State => {_state}");
    }

    private CoreResult BuildResult(string correlationId, bool success, string status, Dictionary<string, string>? data = null)
    {
        data ??= new Dictionary<string, string>();
        data["State"] = _state.ToString();
        return new CoreResult(correlationId, success, status, data);
    }

    private async Task PublishGuiSnapshotAsync(CoreResult result)
    {
        if (_guiPublisher is null)
        {
            return;
        }

        // Always publish a structurally complete lot payload for GUI compatibility.
        GuiLotData? lotData = _currentLotNumber > 0
            ? GuiUdpPublisher.CreateLotData(_currentLotNumber)
            : new GuiLotData
            {
                LotNumber = 0,
                ProductNumber = string.Empty,
                LineId = string.Empty,
                OperatorId = string.Empty,
                Configs = GuiUdpPublisher.CreateLotData(1).Configs
            };

        IReadOnlyList<GuiMeasuredValue>? measuredValues = result.Status.Equals("MeasurementCompleted", StringComparison.OrdinalIgnoreCase)
            ? GuiUdpPublisher.ExtractMeasuredValues(result)
            : null;

        await _guiPublisher.PublishCoreSnapshotAsync(
            result,
            _currentLotNumber > 0,
            lotData,
            _guiCounter,
            measuredValues,
            CancellationToken.None);
    }

    private MeasurementRequest BuildMeasurementRequest(MachineTrigger trigger)
    {
        // Build meter request dynamically based on trigger bits and profile toggles.
        var request = new MeasurementRequest
        {
            CorrelationId = trigger.CorrelationId,
            LotId = trigger.LotId,
            Pattern = trigger.Data.TryGetValue("Pattern", out var pattern) ? pattern ?? string.Empty : string.Empty
        };

        if (IsEnabled(trigger, "TriggerIso"))
        {
            request.Devices.Add(new MeterDeviceRequest
            {
                DeviceId = "SM7110_1",
                DeviceType = MeterDeviceType.Isolation,
                StationNumber = 0,
                TriggerDelayMs = ReadMeterDelay("IsolationDelayMs", 15),
                Channels =
                {
                    new MeterChannelRequest { Name = "ISO", Min = 500.0, Nominal = 800.0, Max = 1500.0 }
                }
            });
        }

        if (IsEnabled(trigger, "TriggerInductanceImpedance"))
        {
            var impedanceDevice = new MeterDeviceRequest
            {
                DeviceId = "IM7581_1",
                DeviceType = MeterDeviceType.InductanceImpedance,
                StationNumber = 1,
                TriggerDelayMs = ReadMeterDelay("ImpedanceDelayMs", 20)
            };
            impedanceDevice.Channels.Add(new MeterChannelRequest { Name = "L", Min = 0.8, Nominal = 1.0, Max = 1.2 });
            impedanceDevice.Channels.Add(new MeterChannelRequest { Name = "Z1", Min = 8.0, Nominal = 10.0, Max = 12.0 });
            if (IsEnabled(trigger, "IncludeZ2"))
            {
                impedanceDevice.Channels.Add(new MeterChannelRequest { Name = "Z2", Min = 8.0, Nominal = 10.0, Max = 12.0 });
            }

            if (IsEnabled(trigger, "IncludeZ3"))
            {
                impedanceDevice.Channels.Add(new MeterChannelRequest { Name = "Z3", Min = 8.0, Nominal = 10.0, Max = 12.0 });
            }

            request.Devices.Add(impedanceDevice);
        }

        if (_options.Profile.MeasureC && IsEnabled(trigger, "TriggerCapacity"))
        {
            var capacityDevice = new MeterDeviceRequest
            {
                DeviceId = "IM7581_2",
                DeviceType = MeterDeviceType.Capacity,
                StationNumber = 2,
                TriggerDelayMs = ReadMeterDelay("CapacityDelayMs", 20)
            };
            capacityDevice.Channels.Add(new MeterChannelRequest { Name = "C1", Min = 80.0, Nominal = 100.0, Max = 120.0 });
            capacityDevice.Channels.Add(new MeterChannelRequest { Name = "C2", Min = 80.0, Nominal = 100.0, Max = 120.0 });
            capacityDevice.Channels.Add(new MeterChannelRequest { Name = "C3", Min = 80.0, Nominal = 100.0, Max = 120.0 });
            request.Devices.Add(capacityDevice);
        }

        if (IsEnabled(trigger, "TriggerResistance"))
        {
            request.Devices.Add(new MeterDeviceRequest
            {
                DeviceId = "RES2329_1",
                DeviceType = MeterDeviceType.Resistance,
                StationNumber = 4,
                TriggerDelayMs = ReadMeterDelay("Resistance1DelayMs", 25),
                Channels =
                {
                    new MeterChannelRequest { Name = "R1", Min = 8.0, Nominal = 10.0, Max = 12.0 }
                }
            });

            request.Devices.Add(new MeterDeviceRequest
            {
                DeviceId = "RES2329_2",
                DeviceType = MeterDeviceType.Resistance,
                StationNumber = 5,
                TriggerDelayMs = ReadMeterDelay("Resistance2DelayMs", 25),
                Channels =
                {
                    new MeterChannelRequest { Name = "R2", Min = 8.0, Nominal = 10.0, Max = 12.0 }
                }
            });
        }

        return request;
    }

    private static bool IsEnabled(MachineTrigger trigger, string key)
    {
        return trigger.Data.TryGetValue(key, out var value) &&
               string.Equals(value, "1", StringComparison.OrdinalIgnoreCase);
    }

    private int ReadMeterDelay(string key, int defaultValue)
    {
        if (_options.MeterModule.Settings.TryGetValue(key, out var value) && int.TryParse(value, out var parsed))
        {
            return parsed;
        }

        return defaultValue;
    }

    private static ulong ToLotNumber(string? lotId)
    {
        if (string.IsNullOrWhiteSpace(lotId))
        {
            return (ulong)DateTimeOffset.UtcNow.ToUnixTimeSeconds();
        }

        var digits = new string(lotId.Where(char.IsDigit).ToArray());
        if (digits.Length > 0 && ulong.TryParse(digits, out var parsed))
        {
            return parsed;
        }

        return (ulong)Math.Abs(lotId.GetHashCode());
    }
}
