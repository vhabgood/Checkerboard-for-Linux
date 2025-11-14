/****************************************************************************
** Meta object code from reading C++ file 'AI.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "AI.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AI.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AI_t {
    QByteArrayData data[90];
    char stringdata0[1064];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AI_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AI_t qt_meta_stringdata_AI = {
    {
QT_MOC_LITERAL(0, 0, 2), // "AI"
QT_MOC_LITERAL(1, 3, 14), // "searchFinished"
QT_MOC_LITERAL(2, 18, 0), // ""
QT_MOC_LITERAL(3, 19, 9), // "moveFound"
QT_MOC_LITERAL(4, 29, 7), // "aborted"
QT_MOC_LITERAL(5, 37, 6), // "CBmove"
QT_MOC_LITERAL(6, 44, 8), // "bestMove"
QT_MOC_LITERAL(7, 53, 6), // "status"
QT_MOC_LITERAL(8, 60, 10), // "gameResult"
QT_MOC_LITERAL(9, 71, 11), // "pdnMoveText"
QT_MOC_LITERAL(10, 83, 11), // "elapsedTime"
QT_MOC_LITERAL(11, 95, 12), // "engineOutput"
QT_MOC_LITERAL(12, 108, 6), // "output"
QT_MOC_LITERAL(13, 115, 11), // "engineError"
QT_MOC_LITERAL(14, 127, 5), // "error"
QT_MOC_LITERAL(15, 133, 17), // "updateToolbarIcon"
QT_MOC_LITERAL(16, 151, 8), // "iconType"
QT_MOC_LITERAL(17, 160, 9), // "iconIndex"
QT_MOC_LITERAL(18, 170, 15), // "logEngineOutput"
QT_MOC_LITERAL(19, 186, 10), // "engineName"
QT_MOC_LITERAL(20, 197, 9), // "totalTime"
QT_MOC_LITERAL(21, 207, 11), // "maxTimeSent"
QT_MOC_LITERAL(22, 219, 8), // "timeUsed"
QT_MOC_LITERAL(23, 228, 8), // "analysis"
QT_MOC_LITERAL(24, 237, 19), // "captureSoundRequest"
QT_MOC_LITERAL(25, 257, 16), // "playSoundRequest"
QT_MOC_LITERAL(26, 274, 14), // "requestNewGame"
QT_MOC_LITERAL(27, 289, 8), // "gametype"
QT_MOC_LITERAL(28, 298, 12), // "updateStatus"
QT_MOC_LITERAL(29, 311, 11), // "changeState"
QT_MOC_LITERAL(30, 323, 8), // "AppState"
QT_MOC_LITERAL(31, 332, 8), // "newState"
QT_MOC_LITERAL(32, 341, 15), // "evaluationReady"
QT_MOC_LITERAL(33, 357, 5), // "score"
QT_MOC_LITERAL(34, 363, 5), // "depth"
QT_MOC_LITERAL(35, 369, 7), // "setMode"
QT_MOC_LITERAL(36, 377, 8), // "AI_State"
QT_MOC_LITERAL(37, 386, 4), // "mode"
QT_MOC_LITERAL(38, 391, 6), // "doWork"
QT_MOC_LITERAL(39, 398, 12), // "requestAbort"
QT_MOC_LITERAL(40, 411, 10), // "loadEngine"
QT_MOC_LITERAL(41, 422, 10), // "enginePath"
QT_MOC_LITERAL(42, 433, 11), // "requestMove"
QT_MOC_LITERAL(43, 445, 8), // "Board8x8"
QT_MOC_LITERAL(44, 454, 5), // "board"
QT_MOC_LITERAL(45, 460, 11), // "colorToMove"
QT_MOC_LITERAL(46, 472, 9), // "timeLimit"
QT_MOC_LITERAL(47, 482, 11), // "abortSearch"
QT_MOC_LITERAL(48, 494, 15), // "internalGetMove"
QT_MOC_LITERAL(49, 510, 5), // "color"
QT_MOC_LITERAL(50, 516, 7), // "maxtime"
QT_MOC_LITERAL(51, 524, 10), // "char[1024]"
QT_MOC_LITERAL(52, 535, 12), // "statusBuffer"
QT_MOC_LITERAL(53, 548, 11), // "QAtomicInt*"
QT_MOC_LITERAL(54, 560, 7), // "playnow"
QT_MOC_LITERAL(55, 568, 4), // "info"
QT_MOC_LITERAL(56, 573, 8), // "moreinfo"
QT_MOC_LITERAL(57, 582, 7), // "CBmove*"
QT_MOC_LITERAL(58, 590, 13), // "startAutoplay"
QT_MOC_LITERAL(59, 604, 16), // "startEngineMatch"
QT_MOC_LITERAL(60, 621, 10), // "totalGames"
QT_MOC_LITERAL(61, 632, 15), // "startRunTestSet"
QT_MOC_LITERAL(62, 648, 15), // "startEngineGame"
QT_MOC_LITERAL(63, 664, 16), // "startAnalyzeGame"
QT_MOC_LITERAL(64, 681, 15), // "startAnalyzePdn"
QT_MOC_LITERAL(65, 697, 16), // "startObserveGame"
QT_MOC_LITERAL(66, 714, 11), // "sendCommand"
QT_MOC_LITERAL(67, 726, 7), // "command"
QT_MOC_LITERAL(68, 734, 8), // "QString&"
QT_MOC_LITERAL(69, 743, 5), // "reply"
QT_MOC_LITERAL(70, 749, 17), // "initEngineProcess"
QT_MOC_LITERAL(71, 767, 17), // "quitEngineProcess"
QT_MOC_LITERAL(72, 785, 19), // "handleAutoplayState"
QT_MOC_LITERAL(73, 805, 22), // "handleEngineMatchState"
QT_MOC_LITERAL(74, 828, 21), // "handleRunTestSetState"
QT_MOC_LITERAL(75, 850, 21), // "handleEngineGameState"
QT_MOC_LITERAL(76, 872, 22), // "handleAnalyzeGameState"
QT_MOC_LITERAL(77, 895, 21), // "handleAnalyzePdnState"
QT_MOC_LITERAL(78, 917, 22), // "handleObserveGameState"
QT_MOC_LITERAL(79, 940, 29), // "move_to_pdn_english_from_list"
QT_MOC_LITERAL(80, 970, 6), // "nmoves"
QT_MOC_LITERAL(81, 977, 8), // "movelist"
QT_MOC_LITERAL(82, 986, 13), // "const CBmove*"
QT_MOC_LITERAL(83, 1000, 4), // "move"
QT_MOC_LITERAL(84, 1005, 5), // "char*"
QT_MOC_LITERAL(85, 1011, 5), // "pdn_c"
QT_MOC_LITERAL(86, 1017, 11), // "setPriority"
QT_MOC_LITERAL(87, 1029, 8), // "priority"
QT_MOC_LITERAL(88, 1038, 11), // "setHandicap"
QT_MOC_LITERAL(89, 1050, 13) // "handicapDepth"

    },
    "AI\0searchFinished\0\0moveFound\0aborted\0"
    "CBmove\0bestMove\0status\0gameResult\0"
    "pdnMoveText\0elapsedTime\0engineOutput\0"
    "output\0engineError\0error\0updateToolbarIcon\0"
    "iconType\0iconIndex\0logEngineOutput\0"
    "engineName\0totalTime\0maxTimeSent\0"
    "timeUsed\0analysis\0captureSoundRequest\0"
    "playSoundRequest\0requestNewGame\0"
    "gametype\0updateStatus\0changeState\0"
    "AppState\0newState\0evaluationReady\0"
    "score\0depth\0setMode\0AI_State\0mode\0"
    "doWork\0requestAbort\0loadEngine\0"
    "enginePath\0requestMove\0Board8x8\0board\0"
    "colorToMove\0timeLimit\0abortSearch\0"
    "internalGetMove\0color\0maxtime\0char[1024]\0"
    "statusBuffer\0QAtomicInt*\0playnow\0info\0"
    "moreinfo\0CBmove*\0startAutoplay\0"
    "startEngineMatch\0totalGames\0startRunTestSet\0"
    "startEngineGame\0startAnalyzeGame\0"
    "startAnalyzePdn\0startObserveGame\0"
    "sendCommand\0command\0QString&\0reply\0"
    "initEngineProcess\0quitEngineProcess\0"
    "handleAutoplayState\0handleEngineMatchState\0"
    "handleRunTestSetState\0handleEngineGameState\0"
    "handleAnalyzeGameState\0handleAnalyzePdnState\0"
    "handleObserveGameState\0"
    "move_to_pdn_english_from_list\0nmoves\0"
    "movelist\0const CBmove*\0move\0char*\0"
    "pdn_c\0setPriority\0priority\0setHandicap\0"
    "handicapDepth"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AI[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      39,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      12,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    7,  209,    2, 0x06 /* Public */,
      11,    1,  224,    2, 0x06 /* Public */,
      13,    1,  227,    2, 0x06 /* Public */,
       3,    1,  230,    2, 0x06 /* Public */,
      15,    2,  233,    2, 0x06 /* Public */,
      18,    6,  238,    2, 0x06 /* Public */,
      24,    0,  251,    2, 0x06 /* Public */,
      25,    0,  252,    2, 0x06 /* Public */,
      26,    1,  253,    2, 0x06 /* Public */,
      28,    1,  256,    2, 0x06 /* Public */,
      29,    1,  259,    2, 0x06 /* Public */,
      32,    2,  262,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      35,    1,  267,    2, 0x0a /* Public */,
      38,    0,  270,    2, 0x0a /* Public */,
      39,    0,  271,    2, 0x0a /* Public */,
      40,    1,  272,    2, 0x0a /* Public */,
      42,    3,  275,    2, 0x0a /* Public */,
      47,    0,  282,    2, 0x0a /* Public */,
      48,    8,  283,    2, 0x0a /* Public */,
      58,    2,  300,    2, 0x0a /* Public */,
      59,    1,  305,    2, 0x0a /* Public */,
      61,    0,  308,    2, 0x0a /* Public */,
      62,    0,  309,    2, 0x0a /* Public */,
      63,    0,  310,    2, 0x0a /* Public */,
      64,    0,  311,    2, 0x0a /* Public */,
      65,    0,  312,    2, 0x0a /* Public */,
      66,    2,  313,    2, 0x0a /* Public */,
      70,    0,  318,    2, 0x0a /* Public */,
      71,    0,  319,    2, 0x0a /* Public */,
      72,    0,  320,    2, 0x0a /* Public */,
      73,    0,  321,    2, 0x0a /* Public */,
      74,    0,  322,    2, 0x0a /* Public */,
      75,    0,  323,    2, 0x0a /* Public */,
      76,    0,  324,    2, 0x0a /* Public */,
      77,    0,  325,    2, 0x0a /* Public */,
      78,    0,  326,    2, 0x0a /* Public */,
      79,    5,  327,    2, 0x0a /* Public */,
      86,    1,  338,    2, 0x0a /* Public */,
      88,    1,  341,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool, QMetaType::Bool, 0x80000000 | 5, QMetaType::QString, QMetaType::Int, QMetaType::QString, QMetaType::Double,    3,    4,    6,    7,    8,    9,   10,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::QString,   14,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   16,   17,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::QString,   19,    9,   20,   21,   22,   23,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   27,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, 0x80000000 | 30,   31,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   33,   34,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 36,   37,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   41,
    QMetaType::Void, 0x80000000 | 43, QMetaType::Int, QMetaType::Double,   44,   45,   46,
    QMetaType::Void,
    QMetaType::Bool, 0x80000000 | 43, QMetaType::Int, QMetaType::Double, 0x80000000 | 51, 0x80000000 | 53, QMetaType::Int, QMetaType::Int, 0x80000000 | 57,   44,   49,   50,   52,   54,   55,   56,    6,
    QMetaType::Void, 0x80000000 | 43, QMetaType::Int,   44,   49,
    QMetaType::Void, QMetaType::Int,   60,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool, QMetaType::QString, 0x80000000 | 68,   67,   69,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 57, 0x80000000 | 82, 0x80000000 | 84, QMetaType::Int,   80,   81,   83,   85,   27,
    QMetaType::Void, QMetaType::Int,   87,
    QMetaType::Void, QMetaType::Int,   89,

       0        // eod
};

void AI::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AI *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->searchFinished((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< CBmove(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5])),(*reinterpret_cast< const QString(*)>(_a[6])),(*reinterpret_cast< double(*)>(_a[7]))); break;
        case 1: _t->engineOutput((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->engineError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->moveFound((*reinterpret_cast< CBmove(*)>(_a[1]))); break;
        case 4: _t->updateToolbarIcon((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->logEngineOutput((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4])),(*reinterpret_cast< double(*)>(_a[5])),(*reinterpret_cast< const QString(*)>(_a[6]))); break;
        case 6: _t->captureSoundRequest(); break;
        case 7: _t->playSoundRequest(); break;
        case 8: _t->requestNewGame((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->updateStatus((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->changeState((*reinterpret_cast< AppState(*)>(_a[1]))); break;
        case 11: _t->evaluationReady((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 12: _t->setMode((*reinterpret_cast< AI_State(*)>(_a[1]))); break;
        case 13: _t->doWork(); break;
        case 14: _t->requestAbort(); break;
        case 15: _t->loadEngine((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 16: _t->requestMove((*reinterpret_cast< const Board8x8(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 17: _t->abortSearch(); break;
        case 18: { bool _r = _t->internalGetMove((*reinterpret_cast< Board8x8(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< char(*)[1024]>(_a[4])),(*reinterpret_cast< QAtomicInt*(*)>(_a[5])),(*reinterpret_cast< int(*)>(_a[6])),(*reinterpret_cast< int(*)>(_a[7])),(*reinterpret_cast< CBmove*(*)>(_a[8])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 19: _t->startAutoplay((*reinterpret_cast< const Board8x8(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 20: _t->startEngineMatch((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 21: _t->startRunTestSet(); break;
        case 22: _t->startEngineGame(); break;
        case 23: _t->startAnalyzeGame(); break;
        case 24: _t->startAnalyzePdn(); break;
        case 25: _t->startObserveGame(); break;
        case 26: { bool _r = _t->sendCommand((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 27: _t->initEngineProcess(); break;
        case 28: _t->quitEngineProcess(); break;
        case 29: _t->handleAutoplayState(); break;
        case 30: _t->handleEngineMatchState(); break;
        case 31: _t->handleRunTestSetState(); break;
        case 32: _t->handleEngineGameState(); break;
        case 33: _t->handleAnalyzeGameState(); break;
        case 34: _t->handleAnalyzePdnState(); break;
        case 35: _t->handleObserveGameState(); break;
        case 36: _t->move_to_pdn_english_from_list((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< CBmove*(*)>(_a[2])),(*reinterpret_cast< const CBmove*(*)>(_a[3])),(*reinterpret_cast< char*(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5]))); break;
        case 37: _t->setPriority((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 38: _t->setHandicap((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 2:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< CBmove >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< CBmove >(); break;
            }
            break;
        case 12:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< AI_State >(); break;
            }
            break;
        case 16:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Board8x8 >(); break;
            }
            break;
        case 18:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Board8x8 >(); break;
            }
            break;
        case 19:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Board8x8 >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AI::*)(bool , bool , CBmove , const QString & , int , const QString & , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AI::searchFinished)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (AI::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AI::engineOutput)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (AI::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AI::engineError)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (AI::*)(CBmove );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AI::moveFound)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (AI::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AI::updateToolbarIcon)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (AI::*)(const QString & , const QString & , double , double , double , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AI::logEngineOutput)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (AI::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AI::captureSoundRequest)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (AI::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AI::playSoundRequest)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (AI::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AI::requestNewGame)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (AI::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AI::updateStatus)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (AI::*)(AppState );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AI::changeState)) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (AI::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AI::evaluationReady)) {
                *result = 11;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AI::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_AI.data,
    qt_meta_data_AI,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AI::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AI.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 39)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 39;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 39)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 39;
    }
    return _id;
}

// SIGNAL 0
void AI::searchFinished(bool _t1, bool _t2, CBmove _t3, const QString & _t4, int _t5, const QString & _t6, double _t7)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t6))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t7))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void AI::engineOutput(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void AI::engineError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void AI::moveFound(CBmove _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void AI::updateToolbarIcon(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void AI::logEngineOutput(const QString & _t1, const QString & _t2, double _t3, double _t4, double _t5, const QString & _t6)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t6))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void AI::captureSoundRequest()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void AI::playSoundRequest()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void AI::requestNewGame(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void AI::updateStatus(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void AI::changeState(AppState _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void AI::evaluationReady(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
