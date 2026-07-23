using System;
using System.Drawing.Text;
using System.IO;
using System.Windows.Input;
using TSMS_Vis.Database;

namespace TSMS_Vis
{
    public class WodiExport
    {
        // helper -> depending on current config !!!
        private const int ISO = 0;
        private const int L = 1;
        private const int Z1 = 2;
        private const int Z2 = 3;
        private const int Z3 = 4;
        private const int C1 = 5;
        private const int C2 = 6;
        private const int C3 = 7;
        private const int R1 = 8;
        private const int R2 = 9;       

        public void writeFile(ulong lot_nr, string path)
        {
            Mouse.OverrideCursor = System.Windows.Input.Cursors.Wait;

            DbHandler db = new DbHandler();
            var data = db.CollectReportData(lot_nr);

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

                file.WriteLine("HiPot_Pass        {0}", data.measData.Count > ISO ? data.measData[ISO].CntPass : -1);
                file.WriteLine("HiPot_Fail_Lo     {0}", data.measData.Count > ISO ? data.measData[ISO].CntLow : -1);
                file.WriteLine("HiPot_Fail_Hi     {0}", data.measData.Count > ISO ? data.measData[ISO].CntHigh : -1);
                file.WriteLine("HiPot_Fail_Sys    {0}", data.measData.Count > ISO ? data.measData[ISO].CntFail : -1);

                file.WriteLine("L_Pass            {0}", data.measData.Count > L ? data.measData[L].CntPass : -1);
                file.WriteLine("L_Fail_Lo         {0}", data.measData.Count > L ? data.measData[L].CntLow : -1);
                file.WriteLine("L_Fail_Hi         {0}", data.measData.Count > L ? data.measData[L].CntHigh : -1);
                file.WriteLine("L_Fail_Sys        {0}", data.measData.Count > L ? data.measData[L].CntFail : -1);
                
                file.WriteLine("Z1_Pass           {0}", data.measData.Count > Z1 ? data.measData[Z1].CntPass : -1);
                file.WriteLine("Z1_Fail_Lo        {0}", data.measData.Count > Z1 ? data.measData[Z1].CntLow : -1);
                file.WriteLine("Z1_Fail_Hi        {0}", data.measData.Count > Z1 ? data.measData[Z1].CntHigh : -1);
                file.WriteLine("Z1_Fail_Sys       {0}", data.measData.Count > Z1 ? data.measData[Z1].CntFail : -1);
                
                file.WriteLine("Z2_Pass           {0}", data.measData.Count > Z2 ? data.measData[Z2].CntPass : -1);
                file.WriteLine("Z2_Fail_Lo        {0}", data.measData.Count > Z2 ? data.measData[Z2].CntLow : -1);
                file.WriteLine("Z2_Fail_Hi        {0}", data.measData.Count > Z2 ? data.measData[Z2].CntHigh : -1);
                file.WriteLine("Z2_Fail_Sys       {0}", data.measData.Count > Z2 ? data.measData[Z2].CntFail : -1);

                file.WriteLine("Z3_Pass           {0}", data.measData.Count > Z3 ? data.measData[Z3].CntPass : -1);
                file.WriteLine("Z3_Fail_Lo        {0}", data.measData.Count > Z3 ? data.measData[Z3].CntLow : -1);
                file.WriteLine("Z3_Fail_Hi        {0}", data.measData.Count > Z3 ? data.measData[Z3].CntHigh : -1);
                file.WriteLine("Z3_Fail_Sys       {0}", data.measData.Count > Z3 ? data.measData[Z3].CntFail : -1);

                file.WriteLine("R_Pass            {0}", "n.a");
                file.WriteLine("R_Fail_Lo         {0}", "n.a");
                file.WriteLine("R_Fail_Hi         {0}", "n.a");
                file.WriteLine("R_Fail_Sys        {0}", "n.a");

                file.WriteLine("R1_Pass           {0}", data.measData.Count > R1 ? data.measData[R1].CntPass : -1);
                file.WriteLine("R1_Fail_Lo        {0}", data.measData.Count > R1 ? data.measData[R1].CntLow : -1);
                file.WriteLine("R1_Fail_Hi        {0}", data.measData.Count > R1 ? data.measData[R1].CntHigh : -1);
                file.WriteLine("R1_Fail_Sys       {0}", data.measData.Count > R1 ? data.measData[R1].CntFail : -1);

                file.WriteLine("R2_Pass           {0}", data.measData.Count > R2 ? data.measData[R2].CntPass : -1);
                file.WriteLine("R2_Fail_Lo        {0}", data.measData.Count > R2 ? data.measData[R2].CntLow : -1);
                file.WriteLine("R2_Fail_Hi        {0}", data.measData.Count > R2 ? data.measData[R2].CntHigh : -1);
                file.WriteLine("R2_Fail_Sys       {0}", data.measData.Count > R2 ? data.measData[R2].CntFail : -1);

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
                
                file.WriteLine("Testsite 3_f1     {0}", data.measData.Count > Z1 ? data.measData[Z1].Frequency : -1);    // [Z1_Frequency]
                file.WriteLine("Testsite 3_f2     {0}", data.measData.Count > Z2 ? data.measData[Z2].Frequency : -1);    // [Z2_Frequency]
                file.WriteLine("Testsite 3_f3     {0}", data.measData.Count > Z3 ? data.measData[Z3].Frequency : -1);    // [Z3_Frequency]
                file.WriteLine("Testsite 3_V      {0}", data.measData.Count > Z1 ? data.measData[Z1].StimuLevel : -1);    // [Z1_StimuLevel]
                file.WriteLine("Testsite 6_f      {0}", "n.a");
                file.WriteLine("Testsite 6_V      {0}", "n.a");
                
                file.WriteLine("HiPot_OG          {0}", data.measData.Count > ISO ? data.measData[ISO].MaxLimit : -1);
                file.WriteLine("HiPot_UG          {0}", data.measData.Count > ISO ? data.measData[ISO].MinLimit : -1);
                file.WriteLine("HiPot_Nominal     {0}", data.measData.Count > ISO ? data.measData[ISO].Nom : -1);
                
                file.WriteLine("L_OG              {0}", data.measData.Count > L ? data.measData[L].MaxLimit : -1);
                file.WriteLine("L_UG              {0}", data.measData.Count > L ? data.measData[L].MinLimit : -1);
                file.WriteLine("L_Nominal         {0}", data.measData.Count > L ? data.measData[L].Nom : -1);

                file.WriteLine("Z1_OG             {0}", data.measData.Count > Z1 ? data.measData[Z1].MaxLimit : -1);
                file.WriteLine("Z1_UG             {0}", data.measData.Count > Z1 ? data.measData[Z1].MinLimit : -1);
                file.WriteLine("Z1_Nominal        {0}", data.measData.Count > Z1 ? data.measData[Z1].Nom : -1);

                file.WriteLine("Z2_OG             {0}", data.measData.Count > Z2 ? data.measData[Z2].MaxLimit : -1);
                file.WriteLine("Z2_UG             {0}", data.measData.Count > Z2 ? data.measData[Z2].MinLimit : -1);
                file.WriteLine("Z2_Nominal        {0}", data.measData.Count > Z2 ? data.measData[Z2].Nom : -1);

                file.WriteLine("Z3_OG             {0}", data.measData.Count > Z3 ? data.measData[Z3].MaxLimit : -1);
                file.WriteLine("Z3_UG             {0}", data.measData.Count > Z3 ? data.measData[Z3].MinLimit : -1);
                file.WriteLine("Z3_Nominal        {0}", data.measData.Count > Z3 ? data.measData[Z3].Nom : -1);

                // todo: What do we do with C ?

                file.WriteLine("R1_OG             {0}", data.measData.Count > R1 ? data.measData[R1].MaxLimit : -1);
                file.WriteLine("R1_UG             {0}", data.measData.Count > R1 ? data.measData[R1].MinLimit : -1);
                file.WriteLine("R1_Nominal        {0}", data.measData.Count > R1 ? data.measData[R1].Nom : -1);

                file.WriteLine("R2_OG             {0}", data.measData.Count > R2 ? data.measData[R2].MaxLimit : -1);
                file.WriteLine("R2_UG             {0}", data.measData.Count > R2 ? data.measData[R2].MinLimit : -1);
                file.WriteLine("R2_Nominal        {0}", data.measData.Count > R2 ? data.measData[R2].Nom : -1);


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
                file.WriteLine("L1-f              {0}", data.measData.Count > L ? data.measData[L].Frequency : -1);         // [L_Frequency]
                file.WriteLine("L-U               {0}", data.measData.Count > L ? data.measData[L].StimuLevel : -1);        // [L_StimuLevel]
                file.WriteLine("Iso-U             {0}", data.measData.Count > ISO ? data.measData[ISO].StimuLevel : -1);    // [Iso_StimuLevel]
                file.WriteLine("ANZ_GUT           {0}", data.MachineCntGood); 
                file.WriteLine("ANZ_FEHLER        {0}", data.CounterSysErr);

            }

            Mouse.OverrideCursor = null;
        }
    }
}
