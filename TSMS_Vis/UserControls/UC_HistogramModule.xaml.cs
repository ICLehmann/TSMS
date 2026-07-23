using System;
using System.Windows;
using System.Windows.Controls;
using TSMS_Comm;
using static ClassLibrary.BasicTypes;

namespace TSMS_Vis.UserControls
{
    /// <summary>
    /// Interaction logic for UC_HistogramModule.xaml
    /// </summary>
    public partial class UC_HistogramModule : UserControl
    {
        public UC_HistogramModule()
        {
            InitializeComponent();

            comboBox.SelectedIndex = 0;
            Type = 0; 
        }

        public void DrawHistogram(string title, HistogramData histogramData)
        {
            lMinY.Content = "Min: " + histogramData.BV_ThresholdMin.ToString("N4");
            lMaxY.Content = "Max: " + histogramData.BV_ThresholdMax.ToString("N4");
            if (histogramData.PATnarrowing)
            {
                lPAT.Content = "PAT (" + histogramData.StartPart.ToString() + ". -)";
                lPAT.Visibility = Visibility.Visible;
            }
            else
            {
                lPAT.Visibility = Visibility.Collapsed;
            }

            hist1.DrawHistogram(title, histogramData);            
        }

        public void SetComboBox(Measurement[] measurements, uint type)
        {
            comboBox.Items.Clear();
            foreach (Measurement m in measurements)
            {
                comboBox.Items.Add(m.Name);
            }
            Type = (int)type;
            OnTypeChanged();
        }

        public static readonly DependencyProperty TypeProperty = DependencyProperty.Register("Type", typeof(int), typeof(UC_HistogramModule), new PropertyMetadata(0, OnTypePropertyChanged));

        private static void OnTypePropertyChanged(DependencyObject sender, DependencyPropertyChangedEventArgs e)
        {
            ((UC_HistogramModule)sender).OnTypeChanged();
        }

        public event EventHandler TypeChanged;
        private void OnTypeChanged()
        {
            int index = Type;
            if (index > comboBox.Items.Count - 1)
                index = comboBox.Items.Count - 1;
        
            Type = index;
            comboBox.SelectedIndex = index;

            if (TypeChanged != null)
                TypeChanged(this, EventArgs.Empty);
        }

        public int Type
        {
            get { return (int)GetValue(TypeProperty); }
            set
            {
                if (value < 0)
                    value = 0;
                SetValue(TypeProperty, value); 
            }
        }

        private void OnSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            Type = comboBox.SelectedIndex;
            hist1.Clear();
        }
    }
}
