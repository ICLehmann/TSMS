using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using TSMS_Vis.Database;
using TSMS_Vis.Report;

namespace TSMS_Vis
{
    /// <summary>
    /// Interaction logic for NarrowingSelectorDialog.xaml
    /// </summary>
    public partial class NarrowingSelectorDialog : Window
    {
        List<ReportNarrowingSection> _list;
        public ReportType SelectedReportType { get; private set; }

        public NarrowingSelectorDialog(List<ReportNarrowingSection> list)
        {
            InitializeComponent();

            _list = list;
            lvSectionToPrint.ItemsSource = _list;
            lvSectionToPrint.Items.Refresh();
        }

        private void lvSectionToPrint_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (lvSectionToPrint.SelectedItem != null)
                selectButton.IsEnabled = true;
            else
                selectButton.IsEnabled = false;
        }

        private void selectCompleteLOT_Click(object sender, RoutedEventArgs e)
        {
            // selection is a complete LOT
            SelectedReportType = new ReportType();
            DialogResult = true;
        }

        private void selectLOTSection_Click(object sender, RoutedEventArgs e)
        {
            if (lvSectionToPrint.SelectedItem != null)
            {
                int start_idx = lvSectionToPrint.SelectedIndex;
                // selection is a narrowed part of the LOT
                SelectedReportType= new ReportType(_list[start_idx]);
                DialogResult = true;
            }
        }
    }
}
