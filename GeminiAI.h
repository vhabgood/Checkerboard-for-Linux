#pragma once

#include <QObject>
#include <QThread>
#include "AIWorker.h"
#include "engine_wrapper.h"
#include "checkers_types.h"

class GeminiAI : public QObject
{
    Q_OBJECT

public:
    explicit GeminiAI(const QString& egdbPath, QObject *parent = nullptr);
    ~GeminiAI();

    void setMode(AI_State mode);
    void setHandicap(int handicap);
    bool sendCommand(const QString& command, QString& reply);
    // User book functions remain as they are not thread-critical
    void addMoveToUserBook(const Board8x8& board, const CBmove& move);
    void deleteCurrentEntry();
    void deleteAllEntriesFromUserBook();
    void navigateToNextEntry();
    void navigateToPreviousEntry();
    void resetNavigation();
    void loadUserBook(const QString& filename);
    void saveUserBook(const QString& filename);
    int getLastEvaluationScore() const { return m_lastEvaluationScore; }
    int getLastSearchDepth() const { return m_lastSearchDepth; }


public slots:
    void init();
    void requestMove(Board8x8 board, int colorToMove, double timeLimit);
    void abortSearch();
    void setExternalEnginePath(const QString& path);
    void setSecondaryExternalEnginePath(const QString& path);
    void setOptions(const CBoptions& options);
    void setEgdbPath(const QString& path);
    void handleWorkerEvaluation(int score, int depth);
    void startAnalyzeGame(const Board8x8& board, int colorToMove);
    void startAutoplay(const Board8x8& board, int colorToMove);
    void startEngineMatch(int numGames, const Board8x8& board, int colorToMove);
    void startRunTestSet(const Board8x8& board, int colorToMove);
    void startAnalyzePdn(const Board8x8& board, int colorToMove);
    void requestAbort();


signals:
    void searchFinished(bool moveFound, bool aborted, const CBmove& bestMove, const QString& statusText, int gameResult, const QString& pdnMoveText, double elapsedTime);
    void engineError(const QString& errorMessage);
    void evaluationReady(int score, int depth);
    // Signal to start a task in the worker thread
    void requestWorkerTask(AI_State task, const Board8x8& board, int color, double maxtime);


private slots:
    void handleWorkerSearchFinished(bool moveFound, bool aborted, const CBmove& bestMove, const QString& statusText, int gameResult, const QString& pdnMoveText, double elapsedTime);

private:
    QThread* m_aiThread;
    AIWorker* m_worker;

    QString m_egdbPath;
    bool m_egdbInitialized;
    int m_maxEGDBPieces;
    AI_State m_mode;
    int m_handicap;
    CBoptions m_options;

    // External engine management
    ExternalEngine *m_primaryExternalEngine;
    ExternalEngine *m_secondaryExternalEngine;
    bool m_useExternalEngine;
    QString m_externalEnginePath;
    QString m_secondaryExternalEnginePath;
    
    int m_lastEvaluationScore;
    int m_lastSearchDepth;
};


