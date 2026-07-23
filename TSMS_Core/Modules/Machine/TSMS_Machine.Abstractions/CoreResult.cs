namespace TSMS.Machine.Abstractions;

public sealed class CoreResult
{
    public CoreResult(string correlationId, bool success, string status, Dictionary<string, string> data)
    {
        CorrelationId = correlationId;
        Success = success;
        Status = status;
        Data = data;
    }

    public string CorrelationId { get; }
    public bool Success { get; }
    public string Status { get; }
    public Dictionary<string, string> Data { get; }

    public static CoreResult Ok(string correlationId, string status)
        => new(correlationId, true, status, new Dictionary<string, string>());

    public static CoreResult Fail(string correlationId, string status)
        => new(correlationId, false, status, new Dictionary<string, string>());
}
