using System;
using System.Collections.Generic;
using System.Data;
using System.Data.SqlClient;
using System.Diagnostics.Eventing.Reader;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Markup;
using TSMS_Vis.Report;

namespace TSMS_Vis.Database
{
    public class PrintLabelInfo
    {
        public long LotNr { get; set; }
        public string ProductNum { get; set; }
        public string LineId { get; set; }
        public int LabelCounter { get; set; }
    }

    public class ReportLots
    {
        public long LotNr { get; set; }
        public string ProductNum { get; set; }
        public string LineId { get; set; }
        public string Status { get; set; }
        public string StartTime { get; set; }
    }

    public class ReportNarrowingSection
    {
        public int StartPart { get; set; }
        public int? EndPart { get; set; }
        public int Mode { get; set; }
        public int? SampleCnt { get; set; }
        public int? LL_Sigma { get; set; }
        public int? UL_Sigma { get; set; }
    }

    public class DbHandler
    {
        private SqlConnection sql_conn = null;

        public DbHandler()
        {
            ConnectToDatabase();
        }

        private void ConnectToDatabase()
        {
            try
            {
                var conBuilder = new SqlConnectionStringBuilder();
                conBuilder.DataSource = ".\\SQLEXPRESS";
                conBuilder.InitialCatalog = "tsms_db";
                conBuilder.IntegratedSecurity = true;

                sql_conn = new SqlConnection(conBuilder.ConnectionString);

                if (sql_conn.State == ConnectionState.Closed)
                    sql_conn.Open();
            }
            catch (Exception ex)
            {
                string msg = "Error while connecting database!";
                Logger.Write(msg);
                MessageBox.Show(msg + "\n" + ex.Message);
            }
        }

        private string ConvertDateTimeFormat(string date_str)
        {
            if (date_str == null || date_str == "")
                return "";

            DateTime date = DateTime.Parse(date_str);
            return date.ToString("yyyy-MM-dd HH:mm:ss");
        }

        private static void SqlDataReaderToCSV(SqlDataReader reader, string filename)
        {
            using (StreamWriter writer = new StreamWriter(filename))
            {
                string[] headers = new string[reader.FieldCount];
                for (int i = 0; i < reader.FieldCount; i++)
                {
                    headers[i] = string.Format("{0}", reader.GetName(i));
                }
                writer.WriteLine(string.Join(";", headers));

                while (reader.Read())
                {
                    string[] row = new string[reader.FieldCount];
                    for (int i = 0; i < reader.FieldCount; i++)
                    {
                        row[i] = string.Format("{0}", reader.GetValue(i));
                    }
                    writer.WriteLine(string.Join(";", row));
                }
            }
        }

        public void ExportMeasurmentsToCSV(ulong lot_nr, string path)
        {
            if (sql_conn.State == ConnectionState.Closed)
                sql_conn.Open();

            string strSQL = "SELECT * FROM [dbo].[Measurements] WHERE [LOT] = " + lot_nr.ToString();
            var cmd = new SqlCommand(strSQL, sql_conn);
            var reader = cmd.ExecuteReader();

            try
            {
                var filename = path + @"\" + String.Format("{0}.csv", lot_nr);
                SqlDataReaderToCSV(reader, filename);
                reader.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'ExportMeasurmentsToCSV'!\n" + ex.Message);
            }
            finally
            {
                reader.Close();
            }
        }

        public int GetLastNarrowingStart(ulong lot_nr)
        {
            if (sql_conn.State == ConnectionState.Closed)
                sql_conn.Open();

            int value = 0; // deafult value

            string strSQL = "SELECT TOP 1 [STARTPART] FROM [dbo].[PAT] WHERE [LOT] = " + lot_nr.ToString() +
                " ORDER BY STARTPART DESC ";
            var cmd = new SqlCommand(strSQL, sql_conn);
            var reader = cmd.ExecuteReader();

            try
            {
                if (reader.Read())
                {
                    value = (int)reader[0];
                }
                reader.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'GetLastNarrowingStart'!\n" + ex.Message);
            }
            finally
            {
                reader.Close();
            }

            return value;
        }

        public List<double> GetMeasurementData(ulong lot_nr, string measurement_name, int? startpart, int? stoppart, string condition = "!= 'fail'")
        {
            if (sql_conn.State == ConnectionState.Closed)
                sql_conn.Open();

            var values = new List<double>();

            StringBuilder sbSQL = new StringBuilder();

            sbSQL.Append("SELECT [" + measurement_name + "] FROM [dbo].[Measurements] WHERE [LOT] = " + lot_nr.ToString());
            if (startpart.HasValue) sbSQL.Append(" AND PART >= " + startpart.ToString());
            if (stoppart.HasValue) sbSQL.Append(" AND PART <= " + stoppart.ToString());
            sbSQL.Append(" AND " + measurement_name + "_Result " + condition);

            var cmd = new SqlCommand(sbSQL.ToString(), sql_conn);
            var reader = cmd.ExecuteReader();

            try
            {
                while (reader.Read())
                {
                    decimal value = (decimal)reader[0];
                    values.Add((double)value);
                }
                reader.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'GetMeasurementData'!\n" + ex.Message);
            }
            finally
            {
                reader.Close();
            }

            return values;
        }

        public void GetMeasurementQualification(ReportData data, int? startpart, int? stoppart)
        {
            if (sql_conn.State == ConnectionState.Closed)
                sql_conn.Open();

            int i;
            StringBuilder sbSQL = new StringBuilder();

            sbSQL.Append("SELECT [PART]");
            for (i = 0; i < data.measData.Count; i++)
            {
                sbSQL.Append(",[" + data.measData[i].Name + "_Result]");
            }
            sbSQL.Append(" FROM [dbo].[Measurements] WHERE [LOT] = " + data.lotHeader.LotNr.ToString());
            if (startpart.HasValue) sbSQL.Append(" AND PART >= " + startpart.ToString());
            if (stoppart.HasValue) sbSQL.Append(" AND PART <= " + stoppart.ToString());

            var cmd = new SqlCommand(sbSQL.ToString(), sql_conn);
            var reader = cmd.ExecuteReader();

            try
            {
                bool good, sysfail, fail;
                string value;
                while (reader.Read())
                {
                    data.Section_TotalParts++;
                    good = true;
                    fail = false;
                    sysfail = false;
                    for (i = 0; i < data.measData.Count; i++)
                    {
                        if (i == 3 || i == 4 || i == 6 || i == 7) // Z2, Z3, C2, C3
                            continue;

                        if (reader[data.measData[i].Name + "_Result"] == DBNull.Value)
                            value = "fail";
                        else
                            value = (string)reader[data.measData[i].Name + "_Result"];

                        switch (value.TrimEnd())
                        {
                            case "pass":
                                data.measData[i].Section_CntPass++;
                                break;
                            case "low":
                                data.measData[i].Section_CntLow++;
                                good = false;
                                fail = true;
                                break;
                            case "high":
                                data.measData[i].Section_CntHigh++;
                                good = false;
                                fail = true;
                                break;
                            default:
                                data.measData[i].Section_CntFail++;
                                good = false;
                                sysfail = true;
                                break;
                        }
                    }
                    if (good) data.Section_TotalGoods++;
                    if (fail) data.Section_TotalFailed++;
                    if (sysfail) data.Section_TotalSysFailed++;
                }
                reader.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'GetMeasurementQualification'!\n" + ex.Message);
            }
            finally
            {
                reader.Close();
            }
        }

        public ReportData CollectReportData(ulong lot_nr, ReportType reporttype)
        {
            ReportData data = new ReportData(lot_nr, reporttype);
            data.narrowingSections = GetNarrowingSections(lot_nr);
            InitReportData(ref data);  // reads measured parameters from the CONFIG table
            AddLotInfoToReportData(ref data); // reads lot data and test limits from the LOT table
            if (reporttype.CompleteLOT == false)
                AddPATInfoToReportData(ref data, reporttype.Section); // reads PAT limits from the PAT table

            for (int i = 0; i < data.measData.Count; i++)
            {
                // reads part data from the MEASUREMENTS table
                if (reporttype.CompleteLOT)
                    data.values[i] = GetMeasurementData(data.lotHeader.LotNr, data.measData[i].Name, null, null, "!= 'error'");
                else
                    data.values[i] = GetMeasurementData(data.lotHeader.LotNr, data.measData[i].Name, reporttype.Section.StartPart, reporttype.Section.EndPart, "!= 'error'");

                if (data.values[i].Count == 0)
                    continue;

                double min = data.values[i].Min();
                double max = data.values[i].Max();
                data.measData[i].MinValue = min; //Math.Round(min, 4); rounded on the UI
                data.measData[i].MaxValue = max; //Math.Round(max, 4); rounded on the UI

                double avg = data.values[i].Average();
                double sum = data.values[i].Sum(d => Math.Pow(d - avg, 2));
                double std_dev = Math.Sqrt(sum / (data.values[i].Count() - 1));
                data.measData[i].StdDev = std_dev; // Math.Round(std_dev, 4); rounded on the UI
            }

            // getsnarrowing section's stat
            if (data.reportType.CompleteLOT)
                GetMeasurementQualification(data, null, null);
            else
                GetMeasurementQualification(data, reporttype.Section.StartPart, reporttype.Section.EndPart);

            return data;
        }

        private void InitReportData(ref ReportData data)
        {
            data.measData.Clear();

            string strSQL = "SELECT * FROM [dbo].[Config]";
            var cmd = new SqlCommand(strSQL, sql_conn);
            var reader = cmd.ExecuteReader();

            try
            {
                while (reader.Read())
                {
                    var meas = new MeasTableEntry();
                    meas.Name = reader[1].ToString().TrimEnd(' ');
                    meas.Unit = reader[2].ToString().TrimEnd(' ');
                    if (meas.Unit == "Ohm")
                        meas.Unit = "Ω";
                    data.measData.Add(meas);
                    data.values.Add(new List<double>());
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'InitReportData'!\n" + ex.Message);
            }
            finally
            {
                reader.Close();
            }
        }

        private void AddLotInfoToReportData(ref ReportData data)
        {
            string strSQL = "SELECT * FROM [dbo].[LOT] WHERE [LOT] = " + data.lotHeader.LotNr.ToString();
            var cmd = new SqlCommand(strSQL, sql_conn);
            var reader = cmd.ExecuteReader();

            try
            {
                while (reader.Read())
                {
                    data.lotHeader.ProductNum = reader["ProductNum"].ToString().TrimEnd(' ');
                    data.lotHeader.LineId = reader["Line"].ToString().TrimEnd(' ');
                    data.lotHeader.TestMachine = reader["TestMachine"].ToString().TrimEnd(' ');
                    data.lotHeader.Status = reader["Status"].ToString().TrimEnd(' ');
                    data.lotHeader.LaserMark = reader["Lasermark"].ToString().TrimEnd(' ');

                    data.lotHeader.StartTime = ConvertDateTimeFormat(reader["LotStart"].ToString().TrimEnd(' '));
                    data.lotHeader.StopTime = ConvertDateTimeFormat(reader["LotStop"].ToString().TrimEnd(' '));

                    //data.TotalParts = (int)reader["TotalParts"];
                    //data.TotalGoods = (int)reader["TotalGoods"];

                    data.MachineCntTotal = (int)reader["MachineCntTotal"];
                    data.MachineCntGood = (int)reader["MachineCntGood"];
                    data.MachineCntGoodBin = (int)reader["MachineCntGoodBin"];
                    data.MachineCntLastBin = (int)reader["MachineCntLastBin"];                   

                    for (int i = 0; i < data.measData.Count; i++)
                    {
                        decimal value = (decimal)reader[data.measData[i].Name + "_Nom"];
                        data.measData[i].Nom = (double)value;
                        value = (decimal)reader[data.measData[i].Name + "_Min"];
                        data.measData[i].MinLimit = (double)value;
                        value = (decimal)reader[data.measData[i].Name + "_Max"];
                        data.measData[i].MaxLimit = (double)value;
                        string prefix = reader[data.measData[i].Name + "_Prefix"].ToString().TrimEnd(' ');
                        data.measData[i].Unit = prefix + data.measData[i].Unit;

                        value = (decimal)reader[data.measData[i].Name + "_StimuLevel"];
                        data.measData[i].StimuLevel = (double)value;
                        data.measData[i].Frequency = (long)reader[data.measData[i].Name + "_Frequency"];
                         
                        data.measData[i].CntPass = (int)reader[data.measData[i].Name + "_Pass"];
                        data.measData[i].CntLow = (int)reader[data.measData[i].Name + "_Low"];
                        data.measData[i].CntHigh = (int)reader[data.measData[i].Name + "_High"];
                        data.measData[i].CntFail = (int)reader[data.measData[i].Name + "_Fail"];
                    }

                    for (int i = 0; i < data.CounterVRB.Length; i++)
                    {
                        data.CounterVRB[i] = (int)reader[TSMS_Consts.CounterVRB_Names[i]];
                    }

                    for (int i = 0; i < data.CounterTRB.Length; i++)
                    {
                        data.CounterTRB[i] = (int)reader[TSMS_Consts.CounterTRB_Names[i]];
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'AddLotInfoToReportData'!\n" + ex.Message);
            }
            finally
            {
                reader.Close();
            }
        }

        private void AddPATInfoToReportData(ref ReportData data, ReportNarrowingSection section)
        {
            string strSQL = "SELECT * FROM [dbo].[PAT] WHERE [LOT] = " + data.lotHeader.LotNr.ToString() + " AND STARTPART = " + section.StartPart;
            var cmd = new SqlCommand(strSQL, sql_conn);
            var reader = cmd.ExecuteReader();

            try
            {
                while (reader.Read())
                {
                    for (int i = 0; i < data.measData.Count; i++)
                    {
                        data.measData[i].PAT_Enabled = (bool)reader[data.measData[i].Name + "_Enabled"];
                        decimal value;
                        value = (decimal)reader[data.measData[i].Name + "_Min"];
                        data.measData[i].PAT_MinLimit = (double)value;
                        value = (decimal)reader[data.measData[i].Name + "_Max"];
                        data.measData[i].PAT_MaxLimit = (double)value;
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'AddLotInfoToReportData'!\n" + ex.Message);
            }
            finally
            {
                reader.Close();
            }
        }

        public List<ReportLots> GetLotsForReport()
        {
            var entries = new List<ReportLots>();
            string strSQL = "SELECT [LOT], [ProductNum], [Line], [Status], [LotStart] FROM [dbo].[LOT] WHERE [ProductNum] != '" +
                TSMS_Consts.DummyTestDataSet + "' AND [ProductNum] != '" + TSMS_Consts.CompensationTestplan + "' ORDER BY [LotStart] DESC";
            var cmd = new SqlCommand(strSQL, sql_conn);
            var reader = cmd.ExecuteReader();

            try
            {
                while (reader.Read())
                {
                    var entry = new ReportLots();
                    entry.LotNr = (long)reader[0];
                    entry.ProductNum = reader[1].ToString().TrimEnd(' ');
                    entry.LineId = reader[2].ToString().TrimEnd(' ');
                    entry.Status = reader[3].ToString().TrimEnd(' ');
                    entry.StartTime = ConvertDateTimeFormat(reader[4].ToString().TrimEnd(' '));
                    entries.Add(entry);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'GetLotsForReport'!\n" + ex.Message);
            }
            finally
            {
                reader.Close();
            }

            return entries;
        }

        public List<ReportLots> GetLotsForReport(DateTime startdate, DateTime enddate, bool finishedlotsonly, string lotstartwith)
        {
            string statuscond = finishedlotsonly ? " AND [Status] = 'FINISHED'" : "";
            var entries = new List<ReportLots>();
            string strSQL = "SELECT [LOT], [ProductNum], [Line], [Status], [LotStart] FROM [dbo].[LOT] WHERE [ProductNum] != '" +
                TSMS_Consts.DummyTestDataSet + "' AND [ProductNum] != '" + TSMS_Consts.CompensationTestplan +
                "' AND [LotStart] >= '" + startdate.ToString("yyyyMMdd") + "' AND [LotStart] < '" + enddate.AddDays(1).ToString("yyyyMMdd") + "'" +
                statuscond;
            if (lotstartwith.Length > 0) strSQL += " AND [LOT] LIKE '" + lotstartwith + "'";
            strSQL+=" ORDER BY [LotStart] DESC";
            var cmd = new SqlCommand(strSQL, sql_conn);
            var reader = cmd.ExecuteReader();

            try
            {
                while (reader.Read())
                {
                    var entry = new ReportLots();
                    entry.LotNr = (long)reader[0];
                    entry.ProductNum = reader[1].ToString().TrimEnd(' ');
                    entry.LineId = reader[2].ToString().TrimEnd(' ');
                    entry.Status = reader[3].ToString().TrimEnd(' ');
                    entry.StartTime = ConvertDateTimeFormat(reader[4].ToString().TrimEnd(' '));
                    entries.Add(entry);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'GetLotsForReport'!\n" + ex.Message);
            }
            finally
            {
                reader.Close();
            }

            return entries;
        }

        private string GetNarrowingModeText(int mode)
        {
            switch (mode)
            {
                case 1:
                    return "SixSigma";
                default:
                    return "Unknown";
            }
        }

        public List<ReportNarrowingSection> GetNarrowingSections(ulong lot_nr)
        {
            var entries = new List<ReportNarrowingSection>();
            string strSQL = "SELECT [STARTPART], [MODE], [SAMPLECNT], [LL_SIGMA], [UL_SIGMA] FROM [dbo].[PAT] WHERE [LOT] = " + lot_nr.ToString() +
                " ORDER BY [STARTPART] ASC";
            var cmd = new SqlCommand(strSQL, sql_conn);
            var reader = cmd.ExecuteReader();

            try
            {
                while (reader.Read())
                {
                    var entry = new ReportNarrowingSection();
                    entry.StartPart = (int)reader[0];
                    entry.Mode = (int)reader[1];
                    entry.SampleCnt = (int)reader[2];
                    entry.LL_Sigma = (int)reader[3];
                    entry.UL_Sigma = (int)reader[4];
                    entries.Add(entry);
                }
                if (entries.Count > 0)
                {
                    // extends narrowing list
                    if (entries[0].StartPart > 1)
                    {
                        // adds the first not narrowed section
                        var entry = new ReportNarrowingSection();
                        entry.StartPart = 1;
                        entry.Mode = 0; // no PAT narrowing
                        entries.Insert(0, entry);  // puts the first not narrowed section
                    }

                    // narrowed sections
                    int n;
                    for (int i = 0; i < entries.Count; i++)
                    {
                        n = i + 1;
                        if (n <= entries.Count - 1)
                        {
                            // adds endpart indices to the list
                            entries[i].EndPart = entries[n].StartPart - 1;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'GetNarrowingSections'!\n" + ex.Message);
            }
            finally
            {
                reader.Close();
            }

            return entries;
        }

        public List<PrintLabelInfo> GetLabelToPrint()
        {
            var entries = new List<PrintLabelInfo>();
            string strSQL = "SELECT [LOT], [ProductNum], [Line], [LabelCounter], [LotStart] FROM [dbo].[LOT] WHERE ProductNum != '" +
                TSMS_Consts.DummyTestDataSet + "' AND ProductNum != '" + TSMS_Consts.CompensationTestplan + "' ORDER BY LotStart DESC";
            var cmd = new SqlCommand(strSQL, sql_conn);
            var reader = cmd.ExecuteReader();

            try
            {
                while (reader.Read())
                {
                    var entry = new PrintLabelInfo();
                    entry.LotNr = (long)reader[0];
                    entry.ProductNum = reader[1].ToString().TrimEnd(' ');
                    entry.LineId = reader[2].ToString().TrimEnd(' ');
                    entry.LabelCounter = (int)reader[3];
                    entries.Add(entry);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'GetLabelToPrint'!\n" + ex.Message);
            }
            finally
            {
                reader.Close();
            }

            return entries;
        }

        public int GetLabelCounter(ulong lot_nr)
        {
            if (sql_conn.State == ConnectionState.Closed)
                sql_conn.Open();

            int Counter = 0;
            string strSQL = "SELECT [LabelCounter] FROM [dbo].[LOT] WHERE [LOT] = " + lot_nr.ToString();
            var cmd = new SqlCommand(strSQL, sql_conn);
            var reader = cmd.ExecuteReader();

            try
            {
                while (reader.Read())
                {
                    Counter = (int)reader[0];
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'GetLabelCounter'!\n" + ex.Message);
            }
            finally
            {
                reader.Close();
            }

            return Counter;
        }

        public void UpdateLabelCounter(ulong lot_nr, int label_cnt)
        {
            string strSQL = "UPDATE [dbo].[LOT] SET [LabelCounter] = " + label_cnt.ToString();
            strSQL += " WHERE [LOT] = " + lot_nr.ToString();
            var cmd = new SqlCommand(strSQL, sql_conn);
            try
            {
                cmd.ExecuteNonQuery();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'UpdateLabelCounter'!\n" + ex.Message);
            }

        }
    }
}
