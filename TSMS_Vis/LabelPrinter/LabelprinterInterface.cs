using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TSMS_Vis
{
    internal interface LabelPrinterInterface
    {
        bool PrintLabel(ulong LotNum, string ProdNum, int LabelCounter);
    }

    internal static class LabelPrinter
    {
        internal static LabelPrinterInterface GetPrinter()
        {
            var ini_file = new IniFile(TSMS_Consts.INI_FILE);
            string labelprintertype = ini_file.Read("LabelPrinter", "GUI");

            LabelPrinterInterface printer;

            switch (labelprintertype.Trim().ToUpper())
            {
                case "SATO":
                    printer = new Sato();
                    break;
                case "ZEBRA":
                    printer = new Zebra();
                    break;
                default:
                    printer = new Sato();
                    break;
            }

            return printer;
        }
    }
}
