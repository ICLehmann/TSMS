using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace TSMS_Vis.Helper
{
    internal static class AppHelper
    {
        public static string ProductName
        {
            get
            {
                return "TSMS";
            }
        }

        public static string GetVersionShortString()
        {
            //Build version 
            System.Reflection.AssemblyName MyInfo = Application.ResourceAssembly.GetName(false);

            return "v" + MyInfo.Version.Major.ToString() + "." + MyInfo.Version.Minor.ToString();
        }

        public static string GetVersionString()
        {
            //Build version 
            System.Reflection.AssemblyName MyInfo = Application.ResourceAssembly.GetName(false);

            return "v" + MyInfo.Version.Major.ToString() + "." + MyInfo.Version.Minor.ToString() + "." + MyInfo.Version.MajorRevision.ToString() + "." + MyInfo.Version.MinorRevision.ToString();
        }

        public static void ErrMsg(Exception ex)
        {
            MessageBox.Show(ex.GetBaseException().Message, ProductName, MessageBoxButton.OK, MessageBoxImage.Error);
        }

        public static void ErrMsg(string strErrorMessage)
        {
            MessageBox.Show(strErrorMessage, ProductName, MessageBoxButton.OK, MessageBoxImage.Error);
        }

        public static void InfoMsg(string Message)
        {
            MessageBox.Show(Message, ProductName, MessageBoxButton.OK, MessageBoxImage.Information);
        }

        public static bool QuestionMsg(string Message)
        {
            return MessageBox.Show(Message, ProductName, MessageBoxButton.YesNo, MessageBoxImage.Question) == MessageBoxResult.Yes;
        }

        public static bool QuestionErrorMsg(string Message)
        {
            return MessageBox.Show(Message, ProductName, MessageBoxButton.YesNo, MessageBoxImage.Error) == MessageBoxResult.Yes;
        }
    }
}
