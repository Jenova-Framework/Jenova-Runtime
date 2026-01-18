#pragma once

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

// Jenova Tiny Profiler Definition
class JenovaTinyProfiler
{
public:
    static void CreateCheckpoint(const std::string& checkPointName);
    static double GetCheckpointTime(const std::string& checkPointName);
    static void DeleteCheckpoint(const std::string& checkPointName);
    static double GetCheckpointTimeAndRestart(const std::string& checkPointName);
    static double GetCheckpointTimeAndDispose(const std::string& checkPointName);

private:
    inline static std::unordered_map<std::string, jenova::SteadyTimePoint> checkpoints;
};