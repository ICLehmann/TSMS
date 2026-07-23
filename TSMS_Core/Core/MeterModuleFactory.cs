using System.Reflection;
using TSMS.Meter.Abstractions;

namespace TSMS.Core;

public static class MeterModuleFactory
{
    public static IMeterModule Create(ModuleOptions options)
    {
        var assemblyPath = ResolveAssemblyPath(options.AssemblyPath);

        if (!File.Exists(assemblyPath))
        {
            throw new FileNotFoundException($"Meter module not found: {assemblyPath}");
        }

        var assembly = Assembly.LoadFrom(assemblyPath);
        var moduleType = assembly.GetType(options.TypeName, throwOnError: true);

        if (moduleType is null)
        {
            throw new InvalidOperationException($"Meter module type not found: {options.TypeName}");
        }

        var instance = Activator.CreateInstance(moduleType);
        if (instance is not IMeterModule meterModule)
        {
            throw new InvalidCastException($"Type {options.TypeName} does not implement IMeterModule.");
        }

        return meterModule;
    }

    private static string ResolveAssemblyPath(string assemblyPath)
    {
        if (string.IsNullOrWhiteSpace(assemblyPath))
        {
            throw new InvalidOperationException("MeterModule.AssemblyPath is empty.");
        }

        if (Path.IsPathRooted(assemblyPath))
        {
            return assemblyPath;
        }

        return Path.Combine(AppContext.BaseDirectory, assemblyPath);
    }
}
