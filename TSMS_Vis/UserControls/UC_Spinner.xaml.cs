using System.Windows;
using System.Windows.Controls;

namespace TSMS_Vis.UserControls
{
    /// <summary>
    /// Interaktionslogik für UC_Spinner.xaml
    /// </summary>
    public partial class UC_Spinner : UserControl
    {
        public UC_Spinner()
        {
            InitializeComponent();
        }

        
        public static readonly DependencyProperty ShowProperty = DependencyProperty.Register("Show", typeof(bool), typeof(UC_Spinner));

        public bool Show
        {
            get { return (bool)GetValue(ShowProperty); }
            set
            {
                if (value)
                    SpinnerPath.Visibility = Visibility.Visible;
                else
                    SpinnerPath.Visibility = Visibility.Hidden;
                SetValue(ShowProperty, value);
            }
        }


    }
}
