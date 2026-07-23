namespace TSMS.Machine.Abstractions;

public enum TriggerKind
{
    StartupRequested = 0,
    LotStartRequested = 1,
    LotInProgressConfirmed = 2,
    MeasurementCycleCompleted = 3,
    LotEndRequested = 4,
    LotPauseRequested = 5,
    LotDiscardRequested = 6,
    DummyStartRequested = 7,
    DummyStopRequested = 8,
    CompensationStartRequested = 9,
    CompensationStopRequested = 10,
    ShutdownRequested = 11,
    OperatorCommand = 12
}

public sealed class MachineTrigger
{
    public MachineTrigger(string correlationId, TriggerKind kind, string? lotId = null, Dictionary<string, string?>? data = null)
    {
        CorrelationId = correlationId;
        Kind = kind;
        LotId = lotId;
        Data = data ?? new Dictionary<string, string?>(StringComparer.OrdinalIgnoreCase);
    }

    public string CorrelationId { get; }
    public TriggerKind Kind { get; }
    public string? LotId { get; }
    public Dictionary<string, string?> Data { get; }
}
