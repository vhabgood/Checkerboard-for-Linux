#pragma once

#include <QObject>

// Define AI_State enum for controlling AI operational modes
enum AI_State {
    Idle,
    AnalyzeGame,
    EngineMatch,
    Autoplay,
    RunTestSet,
    AnalyzePdn
};
Q_DECLARE_METATYPE(AI_State)
