using MathNet.Numerics.Distributions;
using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Forms.DataVisualization.Charting;
using TSMS_Comm;
using static ClassLibrary.BasicTypes;

namespace TSMS_Vis.UserControls
{
    /// <summary>
    /// Interaction logic for UC_ProbabilityModule.xaml
    /// </summary>
    public partial class UC_ProbabilityModule : UserControl
    {
        public UC_ProbabilityModule()
        {
            InitializeComponent();

            comboBox.SelectedIndex = 0;
            Type = 0;
        }

        public  bool DrawProbabilityPlot(ProbabilityData data)
        {
            return prob.DrawProbabilityPlot("",data);
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

        public static readonly DependencyProperty TypeProperty = DependencyProperty.Register("Type", typeof(int), typeof(UC_ProbabilityModule), new PropertyMetadata(0, OnTypePropertyChanged));

        private static void OnTypePropertyChanged(DependencyObject sender, DependencyPropertyChangedEventArgs e)
        {
            ((UC_ProbabilityModule)sender).OnTypeChanged();
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

        }
    }

}
