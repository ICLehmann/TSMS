using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TSMS_Vis.Helper
{
    internal static class TSMSFileSystem
    {
        /// <summary>
        /// Create directory if not exist
        /// </summary>
        /// <param name="path">This path will be checked</param>
        /// <param name="defaultpath">Default path if the path is not defined</param>
        public static void CheckDirectory(ref string path, string defaultpath)
        {
            if (path == "")
                path = defaultpath;

            // if direcory is not exist
            if (!Directory.Exists(path))
            {
                // Try to create the directory.
                DirectoryInfo di = Directory.CreateDirectory(path);
            }
        }
    }
}
