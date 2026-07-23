using System.Collections.Generic;
using System.Windows;
using System.Windows.Forms;
using TSMS_Vis.Database;
using System;
using System.Windows.Documents;
using TSMS_Vis.Resources;
using TSMS_Vis.Report;

namespace TSMS_Vis
{
    /// <summary>
    /// Interaction logic for PrintReportDialog.xaml
    /// </summary>
    public partial class PrintReportDialog : Window
    {
        private DbHandler _db = null;
        private List<ReportLots> _list;

        public PrintReportDialog(ref DbHandler db)
        {
            InitializeComponent();
            SetupUI();
            _db = db;
            QueryLOTs();
        }

        private void SetupUI()
        {
            dtStart.SelectedDate = DateTime.Now.AddDays(-7);
            dtEnd.SelectedDate = DateTime.Now;

            var ini_file = new IniFile(TSMS_Consts.INI_FILE);
            string language = ini_file.Read("Language", "Common");
            // shows wodi button in german environment only
            // because there is no wodi in Hungary
            if (language == "de") wodiButton.Visibility = Visibility.Visible; else wodiButton.Visibility= Visibility.Collapsed;
        }

        private void QueryLOTs()
        {
            // queries LOTs by the defined conditions
            _list = _db.GetLotsForReport(dtStart.SelectedDate.Value.Date, dtEnd.SelectedDate.Value.Date, chkQueryFinishedLOTsOnly.IsChecked.Value, txtLOT_Filter.Text);
            lvLabelToPrint.ItemsSource = _list;
            lvLabelToPrint.Items.Refresh();
        }

        private void lvLabelToPrint_SelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs e)
        {
            if (lvLabelToPrint.SelectedItem != null)
            {
                printButton.IsEnabled = true;
                wodiButton.IsEnabled = true;
            }
            else
            {
                printButton.IsEnabled = false;
                wodiButton.IsEnabled = false; ;
            }
        }

        private ReportType QueryNarrowingSections(ulong lot_nr)
        {
            List<ReportNarrowingSection> narrlist = _db.GetNarrowingSections(lot_nr);
            if (narrlist.Count == 0)
            {
                // no narrowing
                return new ReportType();
            }
            else
            {
                // narrowed
                var dialog = new NarrowingSelectorDialog(narrlist);
                dialog.Owner = this;
                if (dialog.ShowDialog() ?? false) return dialog.SelectedReportType; else return new ReportType();
            }
        }

        private void printButton_Click(object sender, RoutedEventArgs e)
        {
            if (lvLabelToPrint.SelectedItem != null)
            {
                int idx = lvLabelToPrint.SelectedIndex;
                var lot_nr = (ulong)_list[idx].LotNr;       // the selected LOT number

                var selectedreporttype = QueryNarrowingSections(lot_nr);

                var ini_file = new IniFile(TSMS_Consts.INI_FILE);
                string path = ini_file.Read("ReportPath", "Common");    // default path for the report saving
                if (path == "")
                    path = @"C:\Users\Public\Documents";

                var dialog = new FolderBrowserDialog();     // user is able to choose another path for the report saving
                dialog.SelectedPath = path;
                if (dialog.ShowDialog(this.GetIWin32Window()) == System.Windows.Forms.DialogResult.OK)
                {
                    Report.Report report = new Report.Report(lot_nr, selectedreporttype);   // generates report for the seleted LOT
                    report.WriteToPdf(dialog.SelectedPath);             // saves report to the selected path

                    if (DialogResult != null)
                        DialogResult = true;
                }
            }
        }

        private void wodiButton_Click(object sender, RoutedEventArgs e)
        {
            if (lvLabelToPrint.SelectedItem != null)
            {
                int idx = lvLabelToPrint.SelectedIndex;
                var lot_nr = (ulong)_list[idx].LotNr;

                var ini_file = new IniFile(TSMS_Consts.INI_FILE);
                string path = ini_file.Read("WodiExportPath", "Common");
                if (path == "")
                    path = @"C:\Users\Public\Documents";

                WodiExport wodiExport = new WodiExport();
                wodiExport.writeFile(lot_nr, path);
            }
        }

        private void btnQueryLOTs_Click(object sender, RoutedEventArgs e)
        {
            QueryLOTs();
        }
    }
}
