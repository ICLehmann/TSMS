using System.Collections.Generic;
using System.Windows;
using TSMS_Vis.Database;

namespace TSMS_Vis
{
    /// <summary>
    /// Interaction logic for RePrintLabelDialog.xaml
    /// </summary>
    public partial class RePrintLabelDialog : Window
    {
        private DbHandler _db = null;
        private List<PrintLabelInfo> _list;

        public RePrintLabelDialog(ref DbHandler db)
        {
            InitializeComponent();
            _db = db;
            _list = db.GetLabelToPrint();
            lvLabelToPrint.ItemsSource = _list;
            lvLabelToPrint.Items.Refresh();
        }

        private void printButton_Click(object sender, RoutedEventArgs e)
        {
            if (lvLabelToPrint.SelectedItem != null)
            {
                int idx = lvLabelToPrint.SelectedIndex;

                var LabelCounter = _list[idx].LabelCounter;
                var LotNum = (ulong)_list[idx].LotNr;
                var ProdNum = _list[idx].ProductNum;
                var printer = LabelPrinter.GetPrinter();
                if (printer.PrintLabel(LotNum, ProdNum, LabelCounter))
                {
                    LabelCounter++;
                    _db.UpdateLabelCounter(LotNum, LabelCounter);
                    DialogResult = true;
                }
                else
                    DialogResult = false;

                Close();
            }
        }

        private void lvLabelToPrint_SelectionChanged(object sender, System.Windows.Controls.SelectionChangedEventArgs e)
        {
            if (lvLabelToPrint.SelectedItem != null) 
            {
                printButton.IsEnabled = true;
            }
            else 
            {
                printButton.IsEnabled = false; 
            }
        }
    }
}
