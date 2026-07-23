using System;
using System.IO;
using System.Windows;
using System.Windows.Input;
using TSMS_Vis.Database;

namespace TSMS_Vis
{
    public class WodiExport
    {
        // helper -> depending on current config !!!
        private int ISO = -1;
        private int L   = -1;
        private int Z1  = -1;
        private int Z2  = -1;
        private int Z3  = -1;
        private int C1  = -1;
        private int C2  = -1;
        private int C3  = -1;
        private int R1  = -1;
        private int R2  = -1;       

        private void CheckAndStoreIndices(Report.ReportData data)
        {
            // check and store indices
            for (int i = 0; i < data.measData.Count; i++)
            {
                if (data.measData[i].Name == "ISO")
                    ISO = i;
                if (data.measData[i].Name == "L")
                    L = i;
                if (data.measData[i].Name == "Z1")
                    Z1 = i;
                if (data.measData[i].Name == "Z2")
                    Z2 = i;
                if (data.measData[i].Name == "Z3")
                    Z3 = i;
                if (data.measData[i].Name == "C1")
                    C1 = i;
                if (data.measData[i].Name == "C2")
                    C2 = i;
                if (data.measData[i].Name == "C3")
                    C3 = i;
                if (data.measData[i].Name == "R1")
                    R1 = i;
                if (data.measData[i].Name == "R2")
                    R2 = i;                
            }
        }

        public void writeFile(ulong lot_nr, string path)
        {
            if (!Directory.Exists(path))
            {
                MessageBox.Show(path + "\n not exits!", "Path error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            Mouse.OverrideCursor = Cursors.Wait;

            DbHandler db = new DbHandler();
            var data = db.CollectReportData(lot_nr, new Report.ReportType()); // always use a full lot data

            CheckAndStoreIndices(data);
            
            var filename = String.Format("{0}.txt", lot_nr);

            using (StreamWriter file = new StreamWriter(path + @"\" + filename))
            {
                file.WriteLine("Lot number        {0}", data.lotHeader.LotNr);
                file.WriteLine("Department        {0}", "Heidenheim");   
                file.WriteLine("Machine number    {0}", data.lotHeader.TestMachine);
                file.WriteLine("Start             {0}", data.lotHeader.StartTime);
                file.WriteLine("End               {0}", data.lotHeader.StopTime);
                file.WriteLine("Part              {0}", data.lotHeader.ProductNum);
                file.WriteLine("Note              {0}", "");
                file.WriteLine("Marking           {0}", data.lotHeader.LaserMark);
                file.WriteLine("Part Count        {0}", data.MachineCntTotal);
                
                file.WriteLine("Vision 1          {0}", data.CounterVRB.Length > 0 ? data.CounterVRB[0] : -1);
                file.WriteLine("Vision 2          {0}", data.CounterVRB.Length > 1 ? data.CounterVRB[1] : -1);
                file.WriteLine("Vision 3          {0}", data.CounterVRB.Length > 2 ? data.CounterVRB[2] : -1);
                file.WriteLine("Vision 4          {0}", data.CounterVRB.Length > 3 ? data.CounterVRB[3] : -1);
                file.WriteLine("Vision 5          {0}", data.CounterVRB.Length > 4 ? data.CounterVRB[4] : -1);
                
                file.WriteLine("Bin1              {0}", data.CounterTRB.Length > 0 ? data.CounterTRB[0] : -1);
                file.WriteLine("Bin2              {0}", data.CounterTRB.Length > 1 ? data.CounterTRB[1] : -1);
                file.WriteLine("Bin3              {0}", data.CounterTRB.Length > 2 ? data.CounterTRB[2] : -1);
                file.WriteLine("Bin4              {0}", data.CounterTRB.Length > 3 ? data.CounterTRB[3] : -1);
                file.WriteLine("Bin5              {0}", data.CounterTRB.Length > 4 ? data.CounterTRB[4] : -1);
                file.WriteLine("Bin6              {0}", data.CounterTRB.Length > 5 ? data.CounterTRB[5] : -1);
                file.WriteLine("Bin7              {0}", data.CounterTRB.Length > 6 ? data.CounterTRB[6] : -1);
                file.WriteLine("Bin8              {0}", data.CounterTRB.Length > 7 ? data.CounterTRB[7] : -1);
                file.WriteLine("Bin9              {0}", data.CounterVRB.Length > 0 ? data.CounterVRB[0] : -1);
                file.WriteLine("Bin10             {0}", data.CounterVRB.Length > 1 ? data.CounterVRB[1] : -1);
                file.WriteLine("Bin11             {0}", data.CounterVRB.Length > 2 ? data.CounterVRB[2] : -1);
                file.WriteLine("Bin12             {0}", data.CounterVRB.Length > 3 ? data.CounterVRB[3] : -1);
                file.WriteLine("Bin13             {0}", data.MachineCntGoodBin);
                file.WriteLine("Bin14             {0}", data.MachineCntLastBin);

                file.WriteLine("Tape1             {0}", "");    // todo JaO
                file.WriteLine("Tape2             {0}", "");    // todo JaO

                file.WriteLine("HiPot_Pass        {0}", ISO > -1 ? data.measData[ISO].CntPass : -1);
                file.WriteLine("HiPot_Fail_Lo     {0}", ISO > -1 ? data.measData[ISO].CntLow : -1);
                file.WriteLine("HiPot_Fail_Hi     {0}", ISO > -1 ? data.measData[ISO].CntHigh : -1);
                file.WriteLine("HiPot_Fail_Sys    {0}", ISO > -1 ? data.measData[ISO].CntFail : -1);

                file.WriteLine("L_Pass            {0}", L > -1 ? data.measData[L].CntPass : -1);
                file.WriteLine("L_Fail_Lo         {0}", L > -1 ? data.measData[L].CntLow : -1);
                file.WriteLine("L_Fail_Hi         {0}", L > -1 ? data.measData[L].CntHigh : -1);
                file.WriteLine("L_Fail_Sys        {0}", L > -1 ? data.measData[L].CntFail : -1);
                
                file.WriteLine("Z1_Pass           {0}", Z1 > -1 ? data.measData[Z1].CntPass : -1);
                file.WriteLine("Z1_Fail_Lo        {0}", Z1 > -1 ? data.measData[Z1].CntLow : -1);
                file.WriteLine("Z1_Fail_Hi        {0}", Z1 > -1 ? data.measData[Z1].CntHigh : -1);
                file.WriteLine("Z1_Fail_Sys       {0}", Z1 > -1 ? data.measData[Z1].CntFail : -1);
                
                file.WriteLine("Z2_Pass           {0}", Z2 > -1 ? data.measData[Z2].CntPass : -1);
                file.WriteLine("Z2_Fail_Lo        {0}", Z2 > -1 ? data.measData[Z2].CntLow : -1);
                file.WriteLine("Z2_Fail_Hi        {0}", Z2 > -1 ? data.measData[Z2].CntHigh : -1);
                file.WriteLine("Z2_Fail_Sys       {0}", Z2 > -1 ? data.measData[Z2].CntFail : -1);

                file.WriteLine("Z3_Pass           {0}", Z3 > -1 ? data.measData[Z3].CntPass : -1);
                file.WriteLine("Z3_Fail_Lo        {0}", Z3 > -1 ? data.measData[Z3].CntLow : -1);
                file.WriteLine("Z3_Fail_Hi        {0}", Z3 > -1 ? data.measData[Z3].CntHigh : -1);
                file.WriteLine("Z3_Fail_Sys       {0}", Z3 > -1 ? data.measData[Z3].CntFail : -1);

                file.WriteLine("R_Pass            {0}", "n.a");
                file.WriteLine("R_Fail_Lo         {0}", "n.a");
                file.WriteLine("R_Fail_Hi         {0}", "n.a");
                file.WriteLine("R_Fail_Sys        {0}", "n.a");

                file.WriteLine("R1_Pass           {0}", R1 > -1 ? data.measData[R1].CntPass : -1);
                file.WriteLine("R1_Fail_Lo        {0}", R1 > -1 ? data.measData[R1].CntLow : -1);
                file.WriteLine("R1_Fail_Hi        {0}", R1 > -1 ? data.measData[R1].CntHigh : -1);
                file.WriteLine("R1_Fail_Sys       {0}", R1 > -1 ? data.measData[R1].CntFail : -1);

                file.WriteLine("R2_Pass           {0}", R2 > -1 ? data.measData[R2].CntPass : -1);
                file.WriteLine("R2_Fail_Lo        {0}", R2 > -1 ? data.measData[R2].CntLow : -1);
                file.WriteLine("R2_Fail_Hi        {0}", R2 > -1 ? data.measData[R2].CntHigh : -1);
                file.WriteLine("R2_Fail_Sys       {0}", R2 > -1 ? data.measData[R2].CntFail : -1);

                file.WriteLine("Loff_Pass         {0}", "n.a");
                file.WriteLine("Loff_Fail_Lo      {0}", "n.a");
                file.WriteLine("Loff_Fail_Hi      {0}", "n.a");
                file.WriteLine("Loff_Fail_Sys     {0}", "n.a");
                file.WriteLine("Bias_Pass         {0}", "n.a");
                file.WriteLine("Bias_Fail_Lo      {0}", "n.a");
                file.WriteLine("Bias_Fail_Hi      {0}", "n.a");
                file.WriteLine("Bias_Fail_Sys     {0}", "n.a");
                file.WriteLine("Lon_Pass          {0}", "n.a");
                file.WriteLine("Lon_Fail_Lo       {0}", "n.a");
                file.WriteLine("Lon_Fail_Hi       {0}", "n.a");
                file.WriteLine("Lon_Fail_Sys      {0}", "n.a");
                file.WriteLine("L_Drop_Pass       {0}", "n.a");
                file.WriteLine("L_Drop_Fail_Lo    {0}", "n.a");
                file.WriteLine("L_Drop_Fail_Hi    {0}", "n.a");
                file.WriteLine("L_Drop_Fail_Sys   {0}", "n.a");
                
                file.WriteLine("Testsite 3_f1     {0}", Z1 > -1 ? data.measData[Z1].Frequency : -1);    // [Z1_Frequency]
                file.WriteLine("Testsite 3_f2     {0}", Z2 > -1 ? data.measData[Z2].Frequency : -1);    // [Z2_Frequency]
                file.WriteLine("Testsite 3_f3     {0}", Z3 > -1 ? data.measData[Z3].Frequency : -1);    // [Z3_Frequency]
                file.WriteLine("Testsite 3_V      {0}", Z1 > -1 ? data.measData[Z1].StimuLevel : -1);    // [Z1_StimuLevel]
                file.WriteLine("Testsite 6_f      {0}", "n.a");
                file.WriteLine("Testsite 6_V      {0}", "n.a");
                
                file.WriteLine("HiPot_OG          {0}", ISO > -1 ? data.measData[ISO].MaxLimit : -1);
                file.WriteLine("HiPot_UG          {0}", ISO > -1 ? data.measData[ISO].MinLimit : -1);
                file.WriteLine("HiPot_Nominal     {0}", ISO > -1 ? data.measData[ISO].Nom : -1);
                
                file.WriteLine("L_OG              {0}", L > -1 ? data.measData[L].MaxLimit : -1);
                file.WriteLine("L_UG              {0}", L > -1 ? data.measData[L].MinLimit : -1);
                file.WriteLine("L_Nominal         {0}", L > -1 ? data.measData[L].Nom : -1);

                file.WriteLine("Z1_OG             {0}", Z1 > -1 ? data.measData[Z1].MaxLimit : -1);
                file.WriteLine("Z1_UG             {0}", Z1 > -1 ? data.measData[Z1].MinLimit : -1);
                file.WriteLine("Z1_Nominal        {0}", Z1 > -1 ? data.measData[Z1].Nom : -1);

                file.WriteLine("Z2_OG             {0}", Z2 > -1 ? data.measData[Z2].MaxLimit : -1);
                file.WriteLine("Z2_UG             {0}", Z2 > -1 ? data.measData[Z2].MinLimit : -1);
                file.WriteLine("Z2_Nominal        {0}", Z2 > -1 ? data.measData[Z2].Nom : -1);

                file.WriteLine("Z3_OG             {0}", Z3 > -1 ? data.measData[Z3].MaxLimit : -1);
                file.WriteLine("Z3_UG             {0}", Z3 > -1 ? data.measData[Z3].MinLimit : -1);
                file.WriteLine("Z3_Nominal        {0}", Z3 > -1 ? data.measData[Z3].Nom : -1);

                // todo: What do we do with C ?

                file.WriteLine("R1_OG             {0}", R1 > -1 ? data.measData[R1].MaxLimit : -1);
                file.WriteLine("R1_UG             {0}", R1 > -1 ? data.measData[R1].MinLimit : -1);
                file.WriteLine("R1_Nominal        {0}", R1 > -1 ? data.measData[R1].Nom : -1);

                file.WriteLine("R2_OG             {0}", R2 > -1 ? data.measData[R2].MaxLimit : -1);
                file.WriteLine("R2_UG             {0}", R2 > -1 ? data.measData[R2].MinLimit : -1);
                file.WriteLine("R2_Nominal        {0}", R2 > -1 ? data.measData[R2].Nom : -1);


                file.WriteLine("Loff_OG           {0}", "n.a");
                file.WriteLine("Loff_UG           {0}", "n.a");
                file.WriteLine("Loff_Nominal      {0}", "n.a");
                file.WriteLine("Bias_OG           {0}", "n.a");
                file.WriteLine("Bias_UG           {0}", "n.a");
                file.WriteLine("Bias_Nominal      {0}", "n.a");
                file.WriteLine("Lon_OG            {0}", "n.a");
                file.WriteLine("Lon_UG            {0}", "n.a");
                file.WriteLine("Lon_Nominal       {0}", "n.a");
                file.WriteLine("L_Drop_OG         {0}", "n.a");
                file.WriteLine("L_Drop_UG         {0}", "n.a");
                file.WriteLine("L_Drop_Nominal    {0}", "n.a");
                file.WriteLine("R-UnsymAkt        {0}", "n.a");
                file.WriteLine("R-Unsym           {0}", "n.a");
                file.WriteLine("RUNSYM            {0}", "n.a");
                file.WriteLine("R-Eineng          {0}", "");
                file.WriteLine("R-EinSigma        {0}", "");
                file.WriteLine("R-EinMetho        {0}", "");
                file.WriteLine("R-EinStk          {0}", "");
                file.WriteLine("R-EinLimH         {0}", "");
                file.WriteLine("R-EinLimL         {0}", "");
                file.WriteLine("L1-f              {0}", L > -1 ? data.measData[L].Frequency : -1);         // [L_Frequency]
                file.WriteLine("L-U               {0}", L > -1 ? data.measData[L].StimuLevel : -1);        // [L_StimuLevel]
                file.WriteLine("Iso-U             {0}", ISO > -1 ? data.measData[ISO].StimuLevel : -1);    // [Iso_StimuLevel]
                file.WriteLine("ANZ_GUT           {0}", data.MachineCntGood); 
                file.WriteLine("ANZ_FEHLER        {0}", data.Section_TotalSysFailed); // changed from CounterSysErr

            }

            Mouse.OverrideCursor = null;
        }
    }
}
