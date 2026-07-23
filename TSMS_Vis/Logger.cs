using System;
using System.IO;
using System.Text;

namespace TSMS_Vis
{
    public static class Logger
    {
        static string path;
        static Logger() 
        {
            try
            {
                var ini_file = new IniFile(TSMS_Consts.INI_FILE);
                path = ini_file.Read("LogPath", "Common");
                if (path == null)
                    path = "./";

                // if direcory is not exist
                if (!Directory.Exists(path))
                {
                    // Try to create the directory.
                    DirectoryInfo di = Directory.CreateDirectory(path);                    
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("Create logger path failed: {0}", e.ToString());
            }
            finally { }
        }

        private static string GetCurrentDate()
        {
            return DateTime.Now.ToString("yyyy-MM-dd");
        }

        private static string GetCurrentTime()
        {
            return DateTime.Now.ToString("HH:mm:ss");
        }

        public static void Write(string logMsg)
        {
            string filename = $"{GetCurrentDate()}_tsms_gui.log";
            using (StreamWriter sw = new StreamWriter(path + @"\" + filename, true, Encoding.UTF8))
            {
                sw.WriteLine($"{GetCurrentTime()}  {logMsg}");
            }
        }
    }

}
