namespace TSMS_Vis.Report
{
    public class ReportLimits
    {
        public double Bv_min { get; set; }
        public double Bv_max { get; set; }
        public double Narrowed_min { get; set; }
        public double Narrowed_max { get; set; }

        public ReportLimits(ReportData data, int i)
        {
            if (i < data.measData.Count)
            {
                bool narrowed = !data.reportType.CompleteLOT && data.measData[i].PAT_Enabled;
                Bv_min = data.measData[i].MinLimit;
                Bv_max = data.measData[i].MaxLimit;
                Narrowed_min = narrowed ? data.measData[i].PAT_MinLimit : data.measData[i].MinLimit;
                Narrowed_max = narrowed ? data.measData[i].PAT_MaxLimit : data.measData[i].MaxLimit;
            }
        }
    }
}
