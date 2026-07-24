using System.Globalization;

namespace TSMS.Meter.Abstractions;

public static class DeviceTextConfigLoader
{
    public static DeviceTextConfig LoadFromFile(string filePath)
    {
        if (!File.Exists(filePath))
        {
            throw new FileNotFoundException($"Device config file not found: {filePath}");
        }

        var config = new DeviceTextConfig
        {
            DeviceName = Path.GetFileNameWithoutExtension(filePath)
        };

        var lines = File.ReadAllLines(filePath);
        for (var i = 0; i < lines.Length; i++)
        {
            var line = StripComment(lines[i]);
            if (string.IsNullOrWhiteSpace(line))
            {
                continue;
            }

            if (line.StartsWith("@value", StringComparison.OrdinalIgnoreCase))
            {
                config.Values.Add(ReadValueBlock(lines, ref i));
                continue;
            }

            if (TryReadSequence(lines, ref i, line, "@initialization", config.InitializationSequence)) continue;
            if (TryReadSequence(lines, ref i, line, "@setup", config.SetupSequence)) continue;
            if (TryReadSequence(lines, ref i, line, "@start_measurement", config.StartMeasurementSequence)) continue;
            if (TryReadSequence(lines, ref i, line, "@get_answer", config.GetAnswerSequence)) continue;
            if (TryReadSequence(lines, ref i, line, "@compensation_init", config.CompensationInitSequence)) continue;
            if (TryReadSequence(lines, ref i, line, "@compensation_open", config.CompensationOpenSequence)) continue;
            if (TryReadSequence(lines, ref i, line, "@compensation_short", config.CompensationShortSequence)) continue;

            if (TryReadInt(line, "@gpib", out var gpib)) config.IsGpib = gpib != 0;
            if (TryReadString(line, "@address", out var address)) config.Address = address;
            if (TryReadInt(line, "@port_or_device", out var portOrDevice)) config.PortOrDevice = portOrDevice;
            if (TryReadInt(line, "@station", out var station)) config.StationNumber = station;
            if (TryReadInt(line, "@trigger_delay", out var triggerDelay)) config.TriggerDelayMs = triggerDelay;
            if (TryReadInt(line, "@wait_after_start", out var waitAfterStart)) config.WaitAfterStartMs = waitAfterStart;
        }

        return config;
    }

    private static DeviceValueDefinition ReadValueBlock(string[] lines, ref int index)
    {
        var valueDef = new DeviceValueDefinition();
        for (index += 1; index < lines.Length; index++)
        {
            var line = StripComment(lines[index]);
            if (string.IsNullOrWhiteSpace(line))
            {
                continue;
            }

            if (line.StartsWith("@end", StringComparison.OrdinalIgnoreCase))
            {
                break;
            }

            if (TryReadString(line, "name", out var name)) valueDef.Name = name;
            if (TryReadString(line, "unit", out var unit)) valueDef.Unit = unit;
            if (TryReadInt(line, "position", out var position)) valueDef.Position = position;
        }

        return valueDef;
    }

    private static bool TryReadSequence(string[] lines, ref int index, string line, string sectionTag, List<string> target)
    {
        if (!line.StartsWith(sectionTag, StringComparison.OrdinalIgnoreCase))
        {
            return false;
        }

        for (index += 1; index < lines.Length; index++)
        {
            var item = StripComment(lines[index]);
            if (string.IsNullOrWhiteSpace(item))
            {
                continue;
            }

            if (item.StartsWith("@end", StringComparison.OrdinalIgnoreCase))
            {
                break;
            }

            target.Add(item);
        }

        return true;
    }

    private static string StripComment(string line)
    {
        var hash = line.IndexOf('#');
        if (hash >= 0)
        {
            line = line[..hash];
        }

        return line.Trim();
    }

    private static bool TryReadInt(string line, string key, out int value)
    {
        value = 0;
        if (!TryReadString(line, key, out var parsed))
        {
            return false;
        }

        return int.TryParse(parsed, NumberStyles.Integer, CultureInfo.InvariantCulture, out value);
    }

    private static bool TryReadString(string line, string key, out string value)
    {
        value = string.Empty;
        var normalized = key.EndsWith(':') ? key : $"{key}:";
        if (!line.StartsWith(normalized, StringComparison.OrdinalIgnoreCase))
        {
            return false;
        }

        value = line[normalized.Length..].Trim();
        return true;
    }
}
