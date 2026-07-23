#pragma once

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace logger
{
    inline std::string logger_path = "C:\\Users\\Public\\Documents\\TSMS_logs";

    inline void setPath(std::string path)
    {
        logger_path = path;
    }

    inline std::string getCurrentDate()
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d");
        return oss.str();
    }

    inline std::string getCurrentTime()
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%X");
        return oss.str();
    }

    inline void WriteLog(std::string logMsg, bool write_to_console = true)
    {
        if (write_to_console)
            std::cout << logMsg << std::endl << std::flush;

        std::string filePath = logger_path + "/" + getCurrentDate() + "_tsms_master.log";
        std::ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
        ofs << getCurrentTime() << "  " << logMsg << '\n';
        ofs.close();
    }
}