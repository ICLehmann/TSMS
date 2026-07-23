using System;
using System.Text;
using TSMS_Vis;

namespace TSMS_Comm
{
    public struct ValueConfig
    {
        public double min;
        public double nom;
        public double max;
        public double offset;
        public string prefix;
        public double error_rate;
        public ulong frequency;
        public double stimuli_level;
        public ushort type;
        // pat related settings
        public byte pat_enabled;
        public double pat_min;
        public double pat_max;
    };

    public struct TSMS_LotData
    {
        private ulong lot_nr;
        private string product_num;
        private string line_id;
        private string operator_id;
        private ushort num_configs;

        private ValueConfig[] valConfig;

        public ulong LotNumber
        {
            get { return lot_nr; }
        }
        public string ProductNumber
        {
            get { return product_num; }
        }

        public string LineID
        {
            get { return line_id; }
        }

        public string OperatorID
        {
            get { return operator_id; }
        }

        public ushort NumConfigs
        {
            get { return num_configs; }
        }

        public ValueConfig[] ValConfig { get => valConfig; }

        public TSMS_LotData(byte[] btArr)
        {
            int n = 1;
            lot_nr = BitConverter.ToUInt64(btArr, n);
            n += 8;
            product_num = Encoding.UTF8.GetString(btArr, n, TSMS_Consts.TELEGRAM_CHAR_LENGTH);
            product_num = product_num.TrimEnd('\0');
            n += TSMS_Consts.TELEGRAM_CHAR_LENGTH;
            line_id = Encoding.UTF8.GetString(btArr, n, TSMS_Consts.TELEGRAM_CHAR_LENGTH);
            line_id = line_id.TrimEnd('\0');
            n += TSMS_Consts.TELEGRAM_CHAR_LENGTH;
            operator_id = Encoding.UTF8.GetString(btArr, n, TSMS_Consts.TELEGRAM_CHAR_LENGTH);
            operator_id = operator_id.TrimEnd('\0');
            n += TSMS_Consts.TELEGRAM_CHAR_LENGTH;

            num_configs = BitConverter.ToUInt16(btArr, n);
            n += 2;
            valConfig = new ValueConfig[num_configs];
            for (int i = 0; i < valConfig.Length; i++)
            {
                valConfig[i].min = BitConverter.ToDouble(btArr, n);
                n += 8;
                valConfig[i].nom = BitConverter.ToDouble(btArr, n);
                n += 8;
                valConfig[i].max = BitConverter.ToDouble(btArr, n);
                n += 8;
                valConfig[i].offset = BitConverter.ToDouble(btArr, n);
                n += 8;
                if (btArr[n] == 181) // µ
                    valConfig[i].prefix = "µ";      //converting doesn't work
                else
                    valConfig[i].prefix = Encoding.UTF8.GetString(btArr, n, 1);
                n += 1;
                valConfig[i].error_rate = BitConverter.ToDouble(btArr, n);
                n += 8;
                valConfig[i].frequency = BitConverter.ToUInt64(btArr, n);
                n += 8;
                valConfig[i].stimuli_level = BitConverter.ToDouble(btArr, n);
                n += 8;
                valConfig[i].type = BitConverter.ToUInt16(btArr, n);
                n += 2;
                // pat related settings
                valConfig[i].pat_enabled = btArr[n++];
                valConfig[i].pat_min = BitConverter.ToDouble(btArr, n);
                n += 8;
                valConfig[i].pat_max = BitConverter.ToDouble(btArr, n);
                n += 8;
            }
        }

    }
}
