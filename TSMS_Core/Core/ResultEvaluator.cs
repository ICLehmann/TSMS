namespace TSMS.Core;

public sealed class ResultEvaluator
{
    private readonly ProfileOptions _profile;
    private static readonly string[] CriticalMeasurementsWithC = { "ISO", "L", "Z1", "C1", "R1", "R2" };
    private static readonly string[] CriticalMeasurementsWithoutC = { "ISO", "L", "Z1", "R1", "R2" };

    public ResultEvaluator(ProfileOptions profile)
    {
        _profile = profile;
    }

    public EvaluationResult Evaluate(MeasurementBatch batch)
    {
        var mode = (_profile.PatEvaluationMode ?? string.Empty).Trim().ToUpperInvariant();
        if (mode == "ALLMEASUREMENTS")
        {
            var allOk = batch.Values.Count > 0 && batch.Values.Values.All(IsPass);
            return new EvaluationResult(allOk, "AllMeasurements");
        }

        var required = _profile.MeasureC ? CriticalMeasurementsWithC : CriticalMeasurementsWithoutC;
        var criticalOk = required.All(key => batch.Values.TryGetValue(key, out var value) && IsPass(value));
        return new EvaluationResult(criticalOk, "CriticalOnly");
    }

    private static bool IsPass(string? value)
    {
        return string.Equals(value, "PASS", StringComparison.OrdinalIgnoreCase);
    }
}

public sealed class EvaluationResult
{
    public EvaluationResult(bool partOk, string rule)
    {
        PartOk = partOk;
        Rule = rule;
    }

    public bool PartOk { get; }
    public string Rule { get; }
}
