using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Forms.DataVisualization.Charting;
using TSMS_Comm;
using static ClassLibrary.BasicTypes;

namespace TSMS_Vis.UserControls
{
    /// <summary>
    /// Interaction logic for UC_TrendModule.xaml
    /// </summary>
    public partial class UC_TrendModule : UserControl
    {
        private const int VALUES_ON_X_AXIS = 100;
        DataPoint emptyPoint = new DataPoint() { IsEmpty = true };

        public UC_TrendModule()
        {
            InitializeComponent();

            comboBox.SelectedIndex = 0;

            chart1.Series[0].ChartType = SeriesChartType.FastLine;
            chart1.Series[0].Color = System.Drawing.Color.Blue;
            // pat limits
            chart1.Series[1].ChartType = SeriesChartType.FastLine;
            chart1.Series[1].Color = System.Drawing.Color.Orange;
            chart1.Series[2].ChartType = SeriesChartType.FastLine;
            chart1.Series[2].Color = System.Drawing.Color.Orange;

            // activate zoom
            ChartArea CA = chart1.ChartAreas[0];
            CA.AxisX.ScaleView.Zoomable = true;
            CA.CursorX.AutoScroll = true;
            CA.CursorX.IsUserSelectionEnabled = true;
            CA.AxisY.ScaleView.Zoomable = true;
            CA.CursorY.AutoScroll = true;
            CA.CursorY.IsUserSelectionEnabled = true;

            CA.BackColor = System.Drawing.Color.FromArgb(255, 255, 210, 210);
            CA.AxisX.LabelStyle.Enabled = false;

            Reset();

        }

        public void AddValue(double value, byte pat_enabled, double pat_min, double pat_max)
        {
            textBox.Text = value.ToString();

            try
            {
                chart1.Series[0].Points.Add(value);
                // pat limits
                if (pat_enabled > 0)
                {
                    chart1.Series[1].Points.Add(pat_min);
                    chart1.Series[2].Points.Add(pat_max);
                }
                else
                {
                    // add invisible empty points if pat is not enabed
                    chart1.Series[1].Points.Add(emptyPoint);
                    chart1.Series[2].Points.Add(emptyPoint);
                }

                ChartArea CA = chart1.ChartAreas[0];
                CA.AxisX.Minimum = CA.AxisX.Maximum - VALUES_ON_X_AXIS;

                chart1.Update();
            }
            catch
            {

            }
        }

        public void ZoomReset()
        {
            chart1.ChartAreas[0].AxisX.ScaleView.ZoomReset(0);
            chart1.ChartAreas[0].AxisY.ScaleView.ZoomReset(0);
        }

        public void Reset()
        {
            textBox.Text = "0";

            chart1.ChartAreas[0].AxisY.Minimum = double.NaN;
            chart1.ChartAreas[0].AxisY.Maximum = double.NaN;
            chart1.ChartAreas[0].AxisY.Interval = 0;
            chart1.ChartAreas[0].AxisY.StripLines.Clear();

            foreach (var series in chart1.Series)
            {
                series.Points.Clear();
            }
        }

        public void SetupChart(double MinY, double MaxY, double nom, string unit)
        {
            Reset();
            lMinY.Content = "Min: " + MinY.ToString();
            lMaxY.Content = "Max: " + MaxY.ToString();
            lUnit.Content = unit;

            double ValueRange = MaxY - MinY;
            if (ValueRange <= 0)
                return;

            double val = ValueRange;
            if (ValueRange < 1)
                val = 1 / ValueRange;
            int multiples_of_10 = 0;
            while (val > 10)
            {
                val /= 10;
                multiples_of_10++;
            }
            double interval = Math.Pow(10, multiples_of_10);
            if (ValueRange < 1)
                interval = 1 / interval;

            double percent_offset = 0.3;
            double _max_y_axis = Math.Ceiling((MaxY + ValueRange * percent_offset) / interval) * interval;
            double _min_y_axis = Math.Floor((MinY - ValueRange * percent_offset) / interval) * interval;
            chart1.ChartAreas[0].AxisY.Minimum = _min_y_axis;
            chart1.ChartAreas[0].AxisY.Maximum = _max_y_axis;

            if ((_max_y_axis - _min_y_axis) / interval > 15)	// not more than x lines
                interval *= 2;
            if ((_max_y_axis - _min_y_axis) / interval < 8)		// not less than x lines
                interval /= 4;

            chart1.ChartAreas[0].AxisY.Interval = interval;

            // green range
            chart1.ChartAreas[0].AxisY.StripLines.Clear();
            StripLine limit = new StripLine();
            limit.Interval = 0;
            limit.IntervalOffset = MinY;
            limit.StripWidth = ValueRange;
            limit.BackColor = System.Drawing.Color.FromArgb(255, 200, 230, 150);
            chart1.ChartAreas[0].AxisY.StripLines.Add(limit);

            double nominal_width = ValueRange * 0.01;
            StripLine nominal = new StripLine();
            nominal.Interval = 0;
            nominal.IntervalOffset = nom - nominal_width / 2.0;
            nominal.StripWidth = nominal_width;
            nominal.BackColor = System.Drawing.Color.FromArgb(125, 255, 0, 0);
            chart1.ChartAreas[0].AxisY.StripLines.Add(nominal);
        }

        public void SetComboBox(Measurement[] measurements, uint type)
        {
            comboBox.Items.Clear();
            foreach (Measurement m in measurements)
            {
                comboBox.Items.Add(m.Name);
            }
            if (type > measurements.Length)
                type = (uint)measurements.Length;
            comboBox.SelectedIndex = 0;
            Type = (int)type;
        }

        public static readonly DependencyProperty TypeProperty = DependencyProperty.Register("Type", typeof(int), typeof(UC_TrendModule), new PropertyMetadata(0, OnTypePropertyChanged));

        private static void OnTypePropertyChanged(DependencyObject sender, DependencyPropertyChangedEventArgs e)
        {
            ((UC_TrendModule)sender).OnTypeChanged();
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
            foreach (var series in chart1.Series)
            {
                series.Points.Clear();
            }
        }
    }
}
