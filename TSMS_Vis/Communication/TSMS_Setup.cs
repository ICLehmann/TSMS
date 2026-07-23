using System;
using System.Text;
using TSMS_Vis;

namespace TSMS_Comm
{
    public struct Measurement
    {
        public string Name;
        public string Unit;
    }

    public struct TSMS_Setup
    {
        private ushort num_measurements;
        private string machine;

        private Measurement[] measurements;

        public ushort NumMeasurements
        {
            get { return num_measurements; }
        }
        public string Machine
        {
            get { return machine; }
        }

        
        public Measurement[] Measurements { get => measurements; }
        
        public TSMS_Setup(byte[] btArr)
        {
            int n = 1;
            machine = Encoding.UTF8.GetString(btArr, n, TSMS_Consts.TELEGRAM_CHAR_LENGTH);
            machine = machine.TrimEnd('\0');
            n += TSMS_Consts.TELEGRAM_CHAR_LENGTH;

            num_measurements = BitConverter.ToUInt16(btArr, n);
            n += 2;

            measurements = new Measurement[num_measurements];
            for (int i = 0; i < measurements.Length; i++)
            {
                measurements[i].Name = Encoding.UTF8.GetString(btArr, n, TSMS_Consts.TELEGRAM_CHAR_LENGTH);
                measurements[i].Name = measurements[i].Name.TrimEnd('\0');
                n += TSMS_Consts.TELEGRAM_CHAR_LENGTH;
            }

            for (int i = 0; i < measurements.Length; i++)
            {
                measurements[i].Unit = Encoding.UTF8.GetString(btArr, n, TSMS_Consts.TELEGRAM_CHAR_LENGTH);
                measurements[i].Unit = measurements[i].Unit.TrimEnd('\0');
                if (measurements[i].Unit == "Ohm")
                    measurements[i].Unit = "Ω";

                n += TSMS_Consts.TELEGRAM_CHAR_LENGTH;
            }
        }
    }
}
