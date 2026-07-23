using System.Collections.Generic;
using System.Reflection;
using System.Windows.Controls;
using System.Windows.Data;
using TSMS_Comm;

namespace TSMS_Vis.UserControls
{
    public class Counter
    {
        public string Name { get; set; }
        public ulong Total { get; set; }
        public ulong Pass { get; set; }
        public ulong Low { get; set; }
        public ulong High { get; set; }
        public ulong Fail { get; set; }
    }

    /// <summary>
    /// Interaction logic for UC_CounterPanel.xaml
    /// </summary>
    public partial class UC_CounterPanel : UserControl
    {
        private readonly List<Counter> cnt_list;
        public UC_CounterPanel()
        {
            InitializeComponent();

            cnt_list = new List<Counter>();
            lvCounters.ItemsSource = cnt_list;
        }

        public void Setup(Measurement[] measurements)
        {
            cnt_list.Clear();
            foreach (Measurement m in measurements)
                cnt_list.Add(new Counter() { Name = m.Name });
        }

        public void SetCounter(TSMS_Counter counter)
        {
            if (cnt_list.Count == 0)
                return;

            tbTotal.Text = counter.CntTotal.ToString();
            tbGood.Text = counter.CntGood.ToString();

            for (int i = 0; i < counter.ResultPass.Length; i++)
            {
                if (cnt_list.Count < i)
                    break;

                cnt_list[i].Pass = counter.ResultPass[i];
                cnt_list[i].Low = counter.ResultLow[i];
                cnt_list[i].High = counter.ResultHigh[i];
                cnt_list[i].Fail = counter.ResultFail[i];
                cnt_list[i].Total = cnt_list[i].Pass + cnt_list[i].Low + cnt_list[i].High + cnt_list[i].Fail;
            }

            lvCounters.Items.Refresh();
        }

        public void Reset()
        {
            tbTotal.Text = "0";
            tbGood.Text = "0";
            for (int i = 0; i < cnt_list.Count; i++)
            {
                cnt_list[i].Pass = 0;
                cnt_list[i].Low = 0;
                cnt_list[i].High = 0;
                cnt_list[i].Fail = 0;
                cnt_list[i].Total = 0;
            }

            lvCounters.Items.Refresh();
        }
    }
}
