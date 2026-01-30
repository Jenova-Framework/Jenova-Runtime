
/*-------------------------------------------------------------+
|                                                              |
|                   _________   ______ _    _____              |
|                  / / ____/ | / / __ \ |  / /   |             |
|             __  / / __/ /  |/ / / / / | / / /| |             |
|            / /_/ / /___/ /|  / /_/ /| |/ / ___ |             |
|            \____/_____/_/ |_/\____/ |___/_/  |_|             |
|                                                              |
|                        Jenova Runtime                        |
|                   Developed by Hamid.Memar                   |
|                                                              |
+-------------------------------------------------------------*/

// Jenova SDK
#include "Jenova.hpp"

// Jenova Tiny Profiler Implementation
void JenovaTinyProfiler::CreateCheckpoint(const std::string& checkPointName)
{
    auto now = std::chrono::steady_clock::now();
    checkpoints[checkPointName] = now;
}
double JenovaTinyProfiler::GetCheckpointTime(const std::string& checkPointName)
{
    auto now = std::chrono::steady_clock::now();
    if (checkpoints.find(checkPointName) != checkpoints.end())
    {
        auto duration = std::chrono::duration<double, std::milli>(now - checkpoints[checkPointName]);
        return duration.count();
    }
    return -1.0;
}
void JenovaTinyProfiler::DeleteCheckpoint(const std::string& checkPointName)
{
    checkpoints.erase(checkPointName);
}
double JenovaTinyProfiler::GetCheckpointTimeAndRestart(const std::string& checkPointName)
{
    double result = -1.0;
    auto now = std::chrono::steady_clock::now();
    if (checkpoints.find(checkPointName) != checkpoints.end())
    {
        auto duration = std::chrono::duration<double, std::milli>(now - checkpoints[checkPointName]);
        result = duration.count();
    }
    checkpoints[checkPointName] = std::chrono::steady_clock::now();
    return result;
}
double JenovaTinyProfiler::GetCheckpointTimeAndDispose(const std::string& checkPointName)
{
    double result = -1.0;
    auto now = std::chrono::steady_clock::now();
    if (checkpoints.find(checkPointName) != checkpoints.end())
    {
        auto duration = std::chrono::duration<double, std::milli>(now - checkpoints[checkPointName]);
        result = duration.count();
    }
    DeleteCheckpoint(checkPointName);
    return result;
}