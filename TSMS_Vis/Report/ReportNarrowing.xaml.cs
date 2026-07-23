using System.Collections.Generic;
using System.Windows.Controls;
using TSMS_Vis.Resources;

namespace TSMS_Vis.Report
{
    public partial class ReportNarrowing : UserControl
    {
        private List<MeasTableEntry> meas_list;
        public ReportNarrowing()
        {
            InitializeComponent();

            meas_list = new List<MeasTableEntry>();
            lvPATTable.ItemsSource = meas_list;
        }

        public void SetData(ReportData data)
        {
            meas_list.Clear();

            for (int i = 0; i < data.measData.Count; i++)
            {
                if (data.measData[i].PAT_Enabled == false ||
                    data.measData[i].Name == "Z2" || data.measData[i].Name == "Z3" ||
                    data.measData[i].Name == "C2" || data.measData[i].Name == "C3")
                        continue;

                var meas = new MeasTableEntry();
                meas.Name = data.measData[i].Name;
                meas.Unit = data.measData[i].Unit;
                meas.MinLimit = data.measData[i].PAT_MinLimit;
                meas.MaxLimit = data.measData[i].PAT_MaxLimit;
                meas_list.Add(meas);
            }
            lvPATTable.Items.Refresh();

            string mode_text = ReportRes.NARROWINGMETHODDATA;
            Converters.PATMode_toUI_String patmode_toui_string = new Converters.PATMode_toUI_String();
            mode_text = mode_text.Replace("<mode>", (string)patmode_toui_string.Convert(data.reportType.Section.Mode, null, null, null));
            mode_text = mode_text.Replace("<LL>", data.reportType.Section.LL_Sigma.ToString());
            mode_text = mode_text.Replace("<UL>", data.reportType.Section.UL_Sigma.ToString());
            PAT_Mode_Label.Content = mode_text;
        }

    }
}