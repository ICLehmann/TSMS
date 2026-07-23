using System.Text.Json;

namespace TSMS.Core;

public static class CoreOptionsLoader
{
    public static CoreOptions Load(string appSettingsPath)
    {
        if (!File.Exists(appSettingsPath))
        {
            throw new FileNotFoundException($"Configuration file not found: {appSettingsPath}");
        }

        var json = File.ReadAllText(appSettingsPath);
        var options = JsonSerializer.Deserialize<CoreOptions>(json, new JsonSerializerOptions
        {
            PropertyNameCaseInsensitive = true
        });

        if (options is null)
        {
            throw new InvalidOperationException("Configuration could not be read.");
        }

        if (string.IsNullOrWhiteSpace(options.MachineModule.TypeName))
        {
            throw new InvalidOperationException("MachineModule.TypeName is empty.");
        }

        if (string.IsNullOrWhiteSpace(options.MeterModule.TypeName))
        {
            throw new InvalidOperationException("MeterModule.TypeName is empty.");
        }

        return options;
    }
}
