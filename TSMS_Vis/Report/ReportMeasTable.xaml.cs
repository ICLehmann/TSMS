using System.Collections.Generic;
using System.Windows.Controls;

namespace TSMS_Vis.Report
{

    /// <summary>
    /// Interaction logic for UC_TestConfigPanel.xaml
    /// </summary>
    public partial class ReportMeasTable : UserControl
    {
        private List<MeasTableEntry> meas_list;
        public ReportMeasTable()
        {
            InitializeComponent();

            meas_list = new List<MeasTableEntry>();
            lvMeasTable.ItemsSource = meas_list;
        }

        public void SetData(ReportData data)
        {
            meas_list.Clear();

            for (int i = 0; i < data.measData.Count; i++)
            {
                if (data.measData[i].Name == "Z2" || data.measData[i].Name == "Z3" ||
                    data.measData[i].Name == "C2" || data.measData[i].Name == "C3")
                    continue;

                var meas = new MeasTableEntry();
                meas.Name = data.measData[i].Name;
                meas.MinLimit = data.measData[i].MinLimit;
                meas.Nom = data.measData[i].Nom;
                meas.MaxLimit = data.measData[i].MaxLimit;
                meas.Unit = data.measData[i].Unit;
                meas.MinValue = data.measData[i].MinValue;
                meas.MaxValue = data.measData[i].MaxValue;
                meas.StdDev = data.measData[i].StdDev;
                meas_list.Add(meas);    
            }
            lvMeasTable.Items.Refresh();
        }

    }
}