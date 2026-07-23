namespace TSMS.Core.GuiUdp;

internal static class GuiUdpConstants
{
    public const int TelegramCharLength = 20;

    public enum TelMasterToUi : byte
    {
        Setup = 32,
        Status = 33,
        Lot = 34,
        Counter = 35,
        MeasValue = 36,
        Machine = 37,
        CompStatus = 38,
        UserMsg = 39,
        PatUpdate = 40
    }

    public enum TelUiToMaster : byte
    {
        UiIsStarted = 32,
        InitRs232 = 33,
        InitDb = 34,
        InitDevices = 35,
        InitIo = 36,
        RestartPat = 37
    }
}
