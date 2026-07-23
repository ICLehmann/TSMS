using System.ComponentModel;
using System.Windows;
using TSMS_Comm;

namespace TSMS_Vis
{
    /// <summary>
    /// Interaction logic for CompWindow.xaml
    /// </summary>
    public partial class CompWindow : Window
    {
        public CompWindow()
        {
            InitializeComponent();
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            e.Cancel = true;  // cancels the window close    
            this.Hide();      // hides the window ->  we want use the intance again
        }

        public void UpdateStatus(TSMS_CompStatus status)
        {
            cbComp1_Label.Content = status.Label[0];
            //cbComp1_Run.IsChecked = status.Comps[0] == 1;
            Spinner1.Show = status.Comps[0] == 1;
            cbComp1_Ok.IsChecked = status.Comps[0] == 2;
            cbComp1_Error.IsChecked = status.Comps[0] == 3;

            cbComp2_Label.Content = status.Label[1];
            //cbComp2_Run.IsChecked = status.Comps[1] == 1;
            Spinner2.Show = status.Comps[1] == 1;
            cbComp2_Ok.IsChecked = status.Comps[1] == 2;
            cbComp2_Error.IsChecked = status.Comps[1] == 3;

            cbComp3_Label.Content = status.Label[2];
            //cbComp3_Run.IsChecked = status.Comps[2] == 1; 
            Spinner3.Show = status.Comps[2] == 1;
            cbComp3_Ok.IsChecked = status.Comps[2] == 2;
            cbComp3_Error.IsChecked = status.Comps[2] == 3;

            cbComp4_Label.Content = status.Label[3];
            //cbComp4_Run.IsChecked = status.Comps[3] == 1;
            Spinner4.Show = status.Comps[3] == 1;
            cbComp4_Ok.IsChecked = status.Comps[3] == 2;
            cbComp4_Error.IsChecked = status.Comps[3] == 3;

            cbComp5_Label.Content = status.Label[4];
            //cbComp5_Run.IsChecked = status.Comps[4] == 1;
            Spinner5.Show = status.Comps[4] == 1;
            cbComp5_Ok.IsChecked = status.Comps[4] == 2;
            cbComp5_Error.IsChecked = status.Comps[4] == 3;

            cbComp6_Label.Content = status.Label[5];
            //cbComp6_Run.IsChecked = status.Comps[5] == 1;
            Spinner6.Show = status.Comps[5] == 1;
            cbComp6_Ok.IsChecked = status.Comps[5] == 2;
            cbComp6_Error.IsChecked = status.Comps[5] == 3;

        }
    }
}
