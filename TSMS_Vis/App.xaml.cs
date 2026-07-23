using System.Globalization;
using System.Threading;
using System.Windows;
using System.Windows.Markup;

namespace TSMS_Vis
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        public int LangIndex = 0;

        public App()
        {
        }

        protected override void OnStartup(StartupEventArgs e)
        {
            var ini_file = new IniFile(TSMS_Consts.INI_FILE);
            var language = ini_file.Read("Language", "Common");
            if (language == "en")
            {
                LangIndex = 0;
                language = "en-EN";
            }
            else if (language == "de")
            {
                LangIndex = 1;
                language = "de-DE";
            }
            else if (language == "hu")
            {
                LangIndex = 2;
                language = "hu-HU";
            }

            var vCulture = new CultureInfo(language);

            Thread.CurrentThread.CurrentCulture = vCulture;
            Thread.CurrentThread.CurrentUICulture = vCulture;
            CultureInfo.DefaultThreadCurrentCulture = vCulture;
            CultureInfo.DefaultThreadCurrentUICulture = vCulture;

            FrameworkElement.LanguageProperty.OverrideMetadata(typeof(FrameworkElement),
                new FrameworkPropertyMetadata(XmlLanguage.GetLanguage(CultureInfo.CurrentCulture.IetfLanguageTag)));

            base.OnStartup(e);
        }
    }
}
