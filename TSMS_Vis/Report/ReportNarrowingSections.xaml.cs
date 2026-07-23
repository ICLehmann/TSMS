using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Xml.Linq;
using TSMS_Comm;
using TSMS_Vis.Database;
using TSMS_Vis.Resources;

namespace TSMS_Vis.Report
{

    public partial class ReportNarrowingSections : UserControl
    {
        public ReportNarrowingSections()
        {
            InitializeComponent();
        }

        public void SetData(ReportData data)
        {
            lvPATTable.ItemsSource = data.narrowingSections;
            lvPATTable.Items.Refresh();
        }
    }
}