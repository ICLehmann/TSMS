using System.Reflection;
using TSMS.Machine.Abstractions;

namespace TSMS.Core;

public static class MachineModuleFactory
{
    public static IMachineModule Create(ModuleOptions options)
    {
        var assemblyPath = ResolveAssemblyPath(options.AssemblyPath);

        if (!File.Exists(assemblyPath))
        {
            throw new FileNotFoundException($"Machine module not found: {assemblyPath}");
        }

        var assembly = Assembly.LoadFrom(assemblyPath);
        var moduleType = assembly.GetType(options.TypeName, throwOnError: true);

        if (moduleType is null)
        {
            throw new InvalidOperationException($"Module type not found: {options.TypeName}");
        }

        var instance = Activator.CreateInstance(moduleType);
        if (instance is not IMachineModule machineModule)
        {
            throw new InvalidCastException($"Type {options.TypeName} does not implement IMachineModule.");
        }

        return machineModule;
    }

    private static string ResolveAssemblyPath(string assemblyPath)
    {
        if (string.IsNullOrWhiteSpace(assemblyPath))
        {
            throw new InvalidOperationException("Module.AssemblyPath is empty.");
        }

        if (Path.IsPathRooted(assemblyPath))
        {
            return assemblyPath;
        }

        return Path.Combine(AppContext.BaseDirectory, assemblyPath);
    }
}
