using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using TSMS_Comm;

namespace TSMS_Vis.UserControls
{
    public class Config
    {
        public string Name { get; set; }
        public double Min { get; set; }
        public double Nom { get; set; }
        public double Max { get; set; }
        public double Offset { get; set; }
        public double ErrorRate { get; set; }
        public ulong Frequency { get; set; }
        public double Voltage { get; set; }
        public string BaseUnit { get; set; }
        public string Unit { get; set; }
        // pat related settings
        public byte PAT_Enabled { get; set; }
        public double? PAT_Min { get; set; }
        public double? PAT_Max { get; set; }
    }

    /// <summary>
    /// Interaction logic for UC_TestConfigPanel.xaml
    /// </summary>
    public partial class UC_TestConfigPanel : UserControl
    {
        private List<Config> config_list;
        public UC_TestConfigPanel()
        {
            InitializeComponent();

            config_list = new List<Config>();
            lvConfig.ItemsSource = config_list;

        }

        public void Setup(Measurement[] measurements)
        {
            config_list.Clear();
            foreach (Measurement m in measurements)
                config_list.Add(new Config() { Name = m.Name, BaseUnit = m.Unit });
        }

        public void SetConfig(TSMS_LotData lot_data)
        {
            for (int i = 0; i < lot_data.ValConfig.Length; i++)
            {
                if (config_list.Count < i)
                    break;

                config_list[i].Min = lot_data.ValConfig[i].min;
                config_list[i].Nom = lot_data.ValConfig[i].nom;
                config_list[i].Max = lot_data.ValConfig[i].max;
                config_list[i].Offset = lot_data.ValConfig[i].offset;
                config_list[i].ErrorRate = lot_data.ValConfig[i].error_rate;
                config_list[i].Frequency = lot_data.ValConfig[i].frequency;
                config_list[i].Voltage = lot_data.ValConfig[i].stimuli_level;
                config_list[i].Unit = lot_data.ValConfig[i].prefix + config_list[i].BaseUnit;
                // pat related settings
                config_list[i].PAT_Enabled = lot_data.ValConfig[i].pat_enabled;
                if (config_list[i].PAT_Enabled != 0)
                {
                    // shows PAT limits if they are enabled
                    config_list[i].PAT_Min = lot_data.ValConfig[i].pat_min;
                    config_list[i].PAT_Max = lot_data.ValConfig[i].pat_max;
                }
            }
            lvConfig.Items.Refresh();
        }

    }
}