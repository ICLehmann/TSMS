using System.Collections.Generic;
using System.Linq;
using System.Windows.Documents;
using TSMS_Vis.Database;

namespace TSMS_Vis.Report
{
    // complete LOT report or partial LOT report
    public class ReportType
    {
        public bool CompleteLOT { get; set; }
        public ReportNarrowingSection Section { get; set; }

        // complete LOT
        public ReportType()
        {
            this.CompleteLOT = true;
        }

        // partial LOT
        public ReportType(ReportNarrowingSection section)
        {
            if (section == null) throw new System.Exception("Section cannot be null");
            this.CompleteLOT = false;
            this.Section = section;
        }
    }
    
    public class LotHeaderData
    {
        public ulong LotNr { get; set; }
        public string ProductNum { get; set; }
        public string LineId { get; set; }
        public string TestMachine { get; set; }
        public string LaserMark { get; set; }
        public string StartTime { get; set; }
        public string StopTime { get; set; }
        public string Status { get; set; }
    }

    public class MeasTableEntry
    {
        public string Name { get; set; }
        public string Unit { get; set; }
        public double Nom { get; set; }
        public double MinLimit { get; set; }
        public double MaxLimit { get; set; }
        public bool PAT_Enabled { get; set; }
        public double PAT_MinLimit { get; set; }
        public double PAT_MaxLimit { get; set; }
        public double MinValue { get; set; }
        public double MaxValue { get; set; }
        public long Frequency { get; set; }
        public double StimuLevel { get; set; }       
        public double StdDev { get; set; }
        //Query from LOT header -> complete LOT stat
        public int CntPass { get; set; }
        public int CntLow { get; set; }
        public int CntHigh { get; set; }
        public int CntFail { get; set; }
        //Calculated for LOT section -> partial LOT stat
        public int Section_CntPass { get; set; }
        public int Section_CntLow { get; set; }
        public int Section_CntHigh { get; set; }
        public int Section_CntFail { get; set; }
    }

    public class ReportData
    {
        public ReportType reportType; // complete or partial report
        public List<ReportNarrowingSection> narrowingSections; //all narrowing sections
        public LotHeaderData lotHeader;
        public List<MeasTableEntry> measData;
        public List<List<double>> values;
        //public int TotalParts = 0; // calculated by PC
        //public int TotalGoods = 0; // calculated by PC
        public int Section_TotalParts=0; // total part for selected section
        public int Section_TotalGoods=0; // total part for selected section
        public int Section_TotalFailed = 0; // total part for selected section
        public int Section_TotalSysFailed = 0; // total part for selected section
        public int MachineCntTotal = 0;  // calculated by PLC
        public int MachineCntGood = 0;  // calculated by PLC
        public int MachineCntGoodBin = 0;  // calculated by PLC: loose parts, not used at ACT1210
        public int MachineCntLastBin = 0;  // calculated by PLC: parts that are not qualified and have to go back to retesting
        public int[] CounterVRB; // Physical Vision Reject Bin
        public int[] CounterTRB; // Physical Tester Reject Bin

        public ReportData(ulong lot_nr, ReportType reporttype)
        {
            reportType = reporttype;
            lotHeader = new LotHeaderData
            {
                LotNr = lot_nr
            };
            measData = new List<MeasTableEntry>();
            values = new List<List<double>>();
            CounterVRB = new int[TSMS_Consts.CounterVRB_Names.Length];
            CounterTRB = new int[TSMS_Consts.CounterTRB_Names.Length];
        }
       
    }
}
