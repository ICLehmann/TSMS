using System.Runtime.Remoting.Activation;
using System.Windows;

namespace TSMS_Vis
{
    /// <summary>
    /// Interaction logic for MessageDialog.xaml
    /// </summary>
    public partial class MessageDialog : Window
    {
        public MessageDialog()
        {
            InitializeComponent();
        }

        public void SetMessage(string message)
        {
            tbMessage.Text = message;
        }

        private void okButton_Click(object sender, RoutedEventArgs e)
        {
            Hide();
        }
    }
}
