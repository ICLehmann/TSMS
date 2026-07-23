using System.Buffers.Binary;
using System.Text;
using TSMS.Machine.Abstractions;

namespace TSMS.Core.GuiUdp;

internal static class GuiUdpTelegramBuilder
{
    private static readonly string[] MeasurementNames = { "Iso", "L", "Z1", "Z2", "Z3", "C1", "C2", "C3", "R1", "R2" };
    private static readonly string[] MeasurementUnits = { "Ohm", "H", "Ohm", "Ohm", "Ohm", "F", "F", "F", "Ohm", "Ohm" };

    public static byte[] BuildSetup(string machineName)
    {
        var buffer = new byte[1 + GuiUdpConstants.TelegramCharLength + 2 +
                              (MeasurementNames.Length * GuiUdpConstants.TelegramCharLength * 2)];
        var offset = 0;
        buffer[offset++] = (byte)GuiUdpConstants.TelMasterToUi.Setup;
        WriteFixedString(buffer, ref offset, machineName, GuiUdpConstants.TelegramCharLength);
        WriteUInt16(buffer, ref offset, (ushort)MeasurementNames.Length);
        foreach (var name in MeasurementNames)
        {
            WriteFixedString(buffer, ref offset, name, GuiUdpConstants.TelegramCharLength);
        }

        foreach (var unit in MeasurementUnits)
        {
            WriteFixedString(buffer, ref offset, unit, GuiUdpConstants.TelegramCharLength);
        }

        return buffer;
    }

    public static byte[] BuildStatus(CoreResult result)
    {
        var buffer = new byte[1 + 5 + 2 + 2 + 2];
        var offset = 0;
        buffer[offset++] = (byte)GuiUdpConstants.TelMasterToUi.Status;
        buffer[offset++] = TryParseState(result);
        buffer[offset++] = result.Success ? (byte)1 : (byte)2;
        buffer[offset++] = 1;
        buffer[offset++] = 1;
        buffer[offset++] = 1;
        WriteUInt16(buffer, ref offset, 24);
        WriteUInt16(buffer, ref offset, 24);
        WriteUInt16(buffer, ref offset, 0);
        return buffer;
    }

    public static byte[] BuildMachineSignals(bool lotInProgress)
    {
        var buffer = new byte[1 + 10];
        var offset = 0;
        buffer[offset++] = (byte)GuiUdpConstants.TelMasterToUi.Machine;
        buffer[offset++] = 1;
        buffer[offset++] = 1;
        buffer[offset++] = lotInProgress ? (byte)1 : (byte)0;
        buffer[offset++] = 0;
        buffer[offset++] = 0;
        buffer[offset++] = 0;
        buffer[offset++] = 0;
        buffer[offset++] = 0;
        buffer[offset++] = 0;
        buffer[offset++] = 0;
        return buffer;
    }

    public static byte[] BuildUserMessage(string message, bool popup)
    {
        var bytes = Encoding.UTF8.GetBytes(message);
        if (bytes.Length > 1024)
        {
            bytes = bytes[..1024];
        }

        var buffer = new byte[1 + 1 + 2 + bytes.Length];
        var offset = 0;
        buffer[offset++] = (byte)GuiUdpConstants.TelMasterToUi.UserMsg;
        buffer[offset++] = popup ? (byte)1 : (byte)0;
        WriteUInt16(buffer, ref offset, (ushort)bytes.Length);
        Array.Copy(bytes, 0, buffer, offset, bytes.Length);
        return buffer;
    }

    public static byte[] BuildLotData(GuiLotData lotData)
    {
        var numConfigs = lotData.Configs.Count;
        var configSize = 76;
        var buffer = new byte[1 + 8 + (GuiUdpConstants.TelegramCharLength * 3) + 2 + (configSize * numConfigs)];
        var offset = 0;
        buffer[offset++] = (byte)GuiUdpConstants.TelMasterToUi.Lot;
        WriteUInt64(buffer, ref offset, lotData.LotNumber);
        WriteFixedString(buffer, ref offset, lotData.ProductNumber, GuiUdpConstants.TelegramCharLength);
        WriteFixedString(buffer, ref offset, lotData.LineId, GuiUdpConstants.TelegramCharLength);
        WriteFixedString(buffer, ref offset, lotData.OperatorId, GuiUdpConstants.TelegramCharLength);
        WriteUInt16(buffer, ref offset, (ushort)numConfigs);

        foreach (var config in lotData.Configs)
        {
            WriteDouble(buffer, ref offset, config.Min);
            WriteDouble(buffer, ref offset, config.Nom);
            WriteDouble(buffer, ref offset, config.Max);
            WriteDouble(buffer, ref offset, config.Offset);
            buffer[offset++] = (byte)config.Prefix;
            WriteDouble(buffer, ref offset, config.ErrorRate);
            WriteUInt64(buffer, ref offset, config.Frequency);
            WriteDouble(buffer, ref offset, config.StimuliLevel);
            WriteUInt16(buffer, ref offset, config.Type);
            buffer[offset++] = config.PatEnabled ? (byte)1 : (byte)0;
            WriteDouble(buffer, ref offset, config.PatMin);
            WriteDouble(buffer, ref offset, config.PatMax);
        }

        return buffer;
    }

    public static byte[] BuildCounter(GuiCounterSnapshot counter)
    {
        var numCounters = counter.Pass.Length;
        var buffer = new byte[1 + 4 + 4 + 2 + (numCounters * 4 * 4)];
        var offset = 0;
        buffer[offset++] = (byte)GuiUdpConstants.TelMasterToUi.Counter;
        WriteUInt32(buffer, ref offset, counter.Total);
        WriteUInt32(buffer, ref offset, counter.Good);
        WriteUInt16(buffer, ref offset, (ushort)numCounters);
        WriteUInt32Array(buffer, ref offset, counter.Pass);
        WriteUInt32Array(buffer, ref offset, counter.Low);
        WriteUInt32Array(buffer, ref offset, counter.High);
        WriteUInt32Array(buffer, ref offset, counter.Fail);
        return buffer;
    }

    public static byte[] BuildMeasuredValue(GuiMeasuredValue value)
    {
        if (value.PatEnabled)
        {
            var bufferPat = new byte[1 + 2 + 8 + 1 + 8 + 8];
            var offsetPat = 0;
            bufferPat[offsetPat++] = (byte)GuiUdpConstants.TelMasterToUi.MeasValue;
            WriteUInt16(bufferPat, ref offsetPat, value.Type);
            WriteDouble(bufferPat, ref offsetPat, value.Value);
            bufferPat[offsetPat++] = 1;
            WriteDouble(bufferPat, ref offsetPat, value.PatMin);
            WriteDouble(bufferPat, ref offsetPat, value.PatMax);
            return bufferPat;
        }

        var buffer = new byte[1 + 2 + 8 + 1];
        var offset = 0;
        buffer[offset++] = (byte)GuiUdpConstants.TelMasterToUi.MeasValue;
        WriteUInt16(buffer, ref offset, value.Type);
        WriteDouble(buffer, ref offset, value.Value);
        buffer[offset++] = 0;
        return buffer;
    }

    private static byte TryParseState(CoreResult result)
    {
        if (!result.Data.TryGetValue("State", out var stateName))
        {
            return 0;
        }

        return stateName.Trim().ToUpperInvariant() switch
        {
            "BOOTING" => 0,
            "READYFORNEWLOT" => 1,
            "WAITUNTILLOTINPROGRESS" => 2,
            "READYTOMEASURE" => 3,
            "MEASUREMENTRUNNING" => 4,
            "MEASUREMENTEVALUATE" => 5,
            "COMPENSATION" => 6,
            "SHUTDOWNCOMMAND" => 7,
            "DEVICEERROR" => 8,
            "RS232ERROR" => 9,
            "IOERROR" => 10,
            "DBERROR" => 11,
            "EXIT" => 12,
            _ => 0
        };
    }

    private static void WriteFixedString(byte[] buffer, ref int offset, string value, int len)
    {
        var temp = new byte[len];
        var src = Encoding.UTF8.GetBytes(value ?? string.Empty);
        var copyLen = Math.Min(src.Length, len);
        Array.Copy(src, 0, temp, 0, copyLen);
        Array.Copy(temp, 0, buffer, offset, len);
        offset += len;
    }

    private static void WriteUInt16(byte[] buffer, ref int offset, ushort value)
    {
        BinaryPrimitives.WriteUInt16LittleEndian(buffer.AsSpan(offset, 2), value);
        offset += 2;
    }

    private static void WriteUInt32(byte[] buffer, ref int offset, uint value)
    {
        BinaryPrimitives.WriteUInt32LittleEndian(buffer.AsSpan(offset, 4), value);
        offset += 4;
    }

    private static void WriteUInt64(byte[] buffer, ref int offset, ulong value)
    {
        BinaryPrimitives.WriteUInt64LittleEndian(buffer.AsSpan(offset, 8), value);
        offset += 8;
    }

    private static void WriteDouble(byte[] buffer, ref int offset, double value)
    {
        BitConverter.TryWriteBytes(buffer.AsSpan(offset, 8), value);
        offset += 8;
    }

    private static void WriteUInt32Array(byte[] buffer, ref int offset, IReadOnlyList<uint> values)
    {
        foreach (var value in values)
        {
            WriteUInt32(buffer, ref offset, value);
        }
    }
}
