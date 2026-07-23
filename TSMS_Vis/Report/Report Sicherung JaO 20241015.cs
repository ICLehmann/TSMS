using System;
using System.Diagnostics;
using System.IO;
using System.IO.Packaging;
using System.Printing;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Xps;
using System.Windows.Xps.Packaging;
using TSMS_Vis.Database;

namespace TSMS_Vis.Report
{
    public class Report
    {
        private ulong _lot_nr;
        private byte[] _xps_bytes;

        public Report(ulong lot_nr)
        {
            _lot_nr = lot_nr;

            Mouse.OverrideCursor = System.Windows.Input.Cursors.Wait;       

            DbHandler db = new DbHandler();
            var reportData = db.CollectReportData(lot_nr);

            var ms = new MemoryStream();
            var package = Package.Open(ms, FileMode.Create);
            var doc = new XpsDocument(package);
            var writer = XpsDocument.CreateXpsDocumentWriter(doc);

            // Setup for writing multiple visuals
            VisualsToXpsDocument visualToXps = (VisualsToXpsDocument)writer.CreateVisualsCollator();

            const int max_pages = 6;
            for (int page = 1; page <= max_pages; page++)
            {
                Window reportPage = null;
                switch (page)
                {
                    case 1:
                        reportPage = new ReportPage1(reportData, page, max_pages);
                        break;
                    case 2:
                        reportPage = new ReportPage2(reportData, page, max_pages);
                        break;
                    case 3:
                        reportPage = new ReportPage3(reportData, page, max_pages);
                        break;
                    case 4:
                        reportPage = new ReportPage4(reportData, page, max_pages);
                        break;
                    case 5:
                        reportPage = new ReportPage5(reportData, page, max_pages);
                        break;
                    case 6:
                        reportPage = new ReportPage6(reportData, page, max_pages);
                        break;
                }

                reportPage.Show();

                int wait_sec = 1;
                var task = Task.Factory.StartNew(() => Thread.Sleep(new TimeSpan(0, 0, wait_sec)));
                Task.WaitAll(new[] { task });

                visualToXps.Write(reportPage);

                reportPage.Close();
            }

            // End writing multiple visuals
            visualToXps.EndBatchWrite();
            doc.Close();
            package.Close();

            Mouse.OverrideCursor = null;

            _xps_bytes = ms.ToArray();
            
            ms.Dispose();
        }

        public void WriteToPdf(string path)
        {
            try
            {
                var filename = path + @"\" + String.Format("{0}.pdf", _lot_nr);
                var title = "TSMS Report";
                PdfFilePrinter.PrintXpsToPdf(_xps_bytes, filename, title);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on 'PrintXpsToPdf'!\n" + ex.Message);
            }
        }

        public void SendToDefaultPrinter()
        {            
            try 
            {
                var printQueue = LocalPrintServer.GetDefaultPrintQueue();
                var job = printQueue.AddJob();
                job.JobStream.Write(_xps_bytes, 0, _xps_bytes.Length);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error on printing report!\n" + ex.Message);
            }
        }

        private void PrintPdfWithAcrobatReader(string filename)
        {
            ProcessStartInfo info = new ProcessStartInfo();
            info.Verb = "print";
            info.FileName = filename;
            info.CreateNoWindow = true;
            info.WindowStyle = ProcessWindowStyle.Hidden;

            Process p = new Process();
            p.StartInfo = info;
            p.Start();

            long ticks = -1;
            while (ticks != p.TotalProcessorTime.Ticks)
            {
                ticks = p.TotalProcessorTime.Ticks;
                Thread.Sleep(2000);
            }

            if (false == p.CloseMainWindow())
                p.Kill();
        }

    }

}
