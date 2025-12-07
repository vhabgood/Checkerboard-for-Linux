#pragma once

#include <QObject>
#include "checkers_types.h" // Assuming bitboard_pos and AI_State are defined here

// Define a simple QObject to act as a signal emitter for AIWorker
class MainController : public QObject
{
    Q_OBJECT
public:
    explicit MainController(QObject *parent = nullptr) : QObject(parent) {}
signals:
    void requestAiSearch(AI_State task, const bitboard_pos& board, int colorToMove, double timeLimit);
};
