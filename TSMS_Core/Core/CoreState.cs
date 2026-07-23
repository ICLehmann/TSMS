namespace TSMS.Core;

public enum CoreState
{
    Booting = 0,
    ReadyForNewLot = 1,
    WaitUntilLotInProgress = 2,
    ReadyToMeasure = 3,
    MeasurementRunning = 4,
    MeasurementEvaluate = 5,
    Compensation = 6,
    ShutdownCommand = 7,
    DeviceError = 8,
    Rs232Error = 9,
    IoError = 10,
    DbError = 11,
    Exit = 12
}
