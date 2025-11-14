/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "MainWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[135];
    char stringdata0[1734];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 20), // "setPrimaryEnginePath"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 4), // "path"
QT_MOC_LITERAL(4, 38, 22), // "setSecondaryEnginePath"
QT_MOC_LITERAL(5, 61, 14), // "changeAppState"
QT_MOC_LITERAL(6, 76, 8), // "AppState"
QT_MOC_LITERAL(7, 85, 8), // "newState"
QT_MOC_LITERAL(8, 94, 16), // "setStatusBarText"
QT_MOC_LITERAL(9, 111, 4), // "text"
QT_MOC_LITERAL(10, 116, 23), // "updateEvaluationDisplay"
QT_MOC_LITERAL(11, 140, 5), // "score"
QT_MOC_LITERAL(12, 146, 20), // "handleSearchFinished"
QT_MOC_LITERAL(13, 167, 9), // "moveFound"
QT_MOC_LITERAL(14, 177, 7), // "aborted"
QT_MOC_LITERAL(15, 185, 6), // "CBmove"
QT_MOC_LITERAL(16, 192, 8), // "bestMove"
QT_MOC_LITERAL(17, 201, 10), // "statusText"
QT_MOC_LITERAL(18, 212, 10), // "gameResult"
QT_MOC_LITERAL(19, 223, 11), // "pdnMoveText"
QT_MOC_LITERAL(20, 235, 11), // "elapsedTime"
QT_MOC_LITERAL(21, 247, 7), // "newGame"
QT_MOC_LITERAL(22, 255, 15), // "exitApplication"
QT_MOC_LITERAL(23, 271, 9), // "game3Move"
QT_MOC_LITERAL(24, 281, 8), // "gameLoad"
QT_MOC_LITERAL(25, 290, 8), // "gameSave"
QT_MOC_LITERAL(26, 299, 8), // "gameInfo"
QT_MOC_LITERAL(27, 308, 11), // "gameAnalyze"
QT_MOC_LITERAL(28, 320, 8), // "gameCopy"
QT_MOC_LITERAL(29, 329, 9), // "gamePaste"
QT_MOC_LITERAL(30, 339, 12), // "gameDatabase"
QT_MOC_LITERAL(31, 352, 8), // "gameFind"
QT_MOC_LITERAL(32, 361, 10), // "gameFindCR"
QT_MOC_LITERAL(33, 372, 13), // "gameFindTheme"
QT_MOC_LITERAL(34, 386, 14), // "gameSaveAsHtml"
QT_MOC_LITERAL(35, 401, 11), // "gameDiagram"
QT_MOC_LITERAL(36, 413, 14), // "gameFindPlayer"
QT_MOC_LITERAL(37, 428, 18), // "gameFenToClipboard"
QT_MOC_LITERAL(38, 447, 20), // "gameFenFromClipboard"
QT_MOC_LITERAL(39, 468, 18), // "gameSelectUserBook"
QT_MOC_LITERAL(40, 487, 12), // "gameReSearch"
QT_MOC_LITERAL(41, 500, 12), // "gameLoadNext"
QT_MOC_LITERAL(42, 513, 16), // "gameLoadPrevious"
QT_MOC_LITERAL(43, 530, 14), // "gameAnalyzePdn"
QT_MOC_LITERAL(44, 545, 17), // "gameSampleDiagram"
QT_MOC_LITERAL(45, 563, 9), // "movesPlay"
QT_MOC_LITERAL(46, 573, 9), // "movesBack"
QT_MOC_LITERAL(47, 583, 12), // "movesForward"
QT_MOC_LITERAL(48, 596, 12), // "movesBackAll"
QT_MOC_LITERAL(49, 609, 15), // "movesForwardAll"
QT_MOC_LITERAL(50, 625, 12), // "movesComment"
QT_MOC_LITERAL(51, 638, 15), // "interruptEngine"
QT_MOC_LITERAL(52, 654, 11), // "abortEngine"
QT_MOC_LITERAL(53, 666, 10), // "levelExact"
QT_MOC_LITERAL(54, 677, 12), // "levelInstant"
QT_MOC_LITERAL(55, 690, 8), // "level01S"
QT_MOC_LITERAL(56, 699, 8), // "level02S"
QT_MOC_LITERAL(57, 708, 8), // "level05S"
QT_MOC_LITERAL(58, 717, 7), // "level1S"
QT_MOC_LITERAL(59, 725, 7), // "level2S"
QT_MOC_LITERAL(60, 733, 7), // "level5S"
QT_MOC_LITERAL(61, 741, 8), // "level10S"
QT_MOC_LITERAL(62, 750, 8), // "level15S"
QT_MOC_LITERAL(63, 759, 8), // "level30S"
QT_MOC_LITERAL(64, 768, 7), // "level1M"
QT_MOC_LITERAL(65, 776, 7), // "level2M"
QT_MOC_LITERAL(66, 784, 7), // "level5M"
QT_MOC_LITERAL(67, 792, 8), // "level15M"
QT_MOC_LITERAL(68, 801, 8), // "level30M"
QT_MOC_LITERAL(69, 810, 13), // "levelInfinite"
QT_MOC_LITERAL(70, 824, 14), // "levelIncrement"
QT_MOC_LITERAL(71, 839, 12), // "levelAddTime"
QT_MOC_LITERAL(72, 852, 17), // "levelSubtractTime"
QT_MOC_LITERAL(73, 870, 8), // "pieceSet"
QT_MOC_LITERAL(74, 879, 16), // "optionsHighlight"
QT_MOC_LITERAL(75, 896, 12), // "optionsSound"
QT_MOC_LITERAL(76, 909, 15), // "optionsPriority"
QT_MOC_LITERAL(77, 925, 12), // "options3Move"
QT_MOC_LITERAL(78, 938, 18), // "optionsDirectories"
QT_MOC_LITERAL(79, 957, 15), // "optionsUserBook"
QT_MOC_LITERAL(80, 973, 22), // "optionsLanguageEnglish"
QT_MOC_LITERAL(81, 996, 22), // "optionsLanguageEspanol"
QT_MOC_LITERAL(82, 1019, 23), // "optionsLanguageItaliano"
QT_MOC_LITERAL(83, 1043, 22), // "optionsLanguageDeutsch"
QT_MOC_LITERAL(84, 1066, 23), // "optionsLanguageFrancais"
QT_MOC_LITERAL(85, 1090, 13), // "displayInvert"
QT_MOC_LITERAL(86, 1104, 14), // "displayNumbers"
QT_MOC_LITERAL(87, 1119, 13), // "displayMirror"
QT_MOC_LITERAL(88, 1133, 8), // "cmNormal"
QT_MOC_LITERAL(89, 1142, 10), // "cmAnalysis"
QT_MOC_LITERAL(90, 1153, 10), // "gotoNormal"
QT_MOC_LITERAL(91, 1164, 10), // "cmAutoplay"
QT_MOC_LITERAL(92, 1175, 9), // "cm2Player"
QT_MOC_LITERAL(93, 1185, 14), // "engineVsEngine"
QT_MOC_LITERAL(94, 1200, 17), // "colorBoardNumbers"
QT_MOC_LITERAL(95, 1218, 14), // "colorHighlight"
QT_MOC_LITERAL(96, 1233, 12), // "bookModeView"
QT_MOC_LITERAL(97, 1246, 11), // "bookModeAdd"
QT_MOC_LITERAL(98, 1258, 14), // "bookModeDelete"
QT_MOC_LITERAL(99, 1273, 12), // "engineSelect"
QT_MOC_LITERAL(100, 1286, 11), // "engineAbout"
QT_MOC_LITERAL(101, 1298, 10), // "engineHelp"
QT_MOC_LITERAL(102, 1309, 13), // "engineOptions"
QT_MOC_LITERAL(103, 1323, 13), // "engineAnalyze"
QT_MOC_LITERAL(104, 1337, 14), // "engineInfinite"
QT_MOC_LITERAL(105, 1352, 12), // "engineResign"
QT_MOC_LITERAL(106, 1365, 10), // "engineDraw"
QT_MOC_LITERAL(107, 1376, 14), // "engineUndoMove"
QT_MOC_LITERAL(108, 1391, 12), // "enginePonder"
QT_MOC_LITERAL(109, 1404, 13), // "cmEngineMatch"
QT_MOC_LITERAL(110, 1418, 12), // "cmAddComment"
QT_MOC_LITERAL(111, 1431, 10), // "engineEval"
QT_MOC_LITERAL(112, 1442, 15), // "cmEngineCommand"
QT_MOC_LITERAL(113, 1458, 12), // "cmRunTestSet"
QT_MOC_LITERAL(114, 1471, 10), // "cmHandicap"
QT_MOC_LITERAL(115, 1482, 9), // "setupMode"
QT_MOC_LITERAL(116, 1492, 10), // "setupClear"
QT_MOC_LITERAL(117, 1503, 10), // "setupBlack"
QT_MOC_LITERAL(118, 1514, 10), // "setupWhite"
QT_MOC_LITERAL(119, 1525, 7), // "setupCc"
QT_MOC_LITERAL(120, 1533, 8), // "helpHelp"
QT_MOC_LITERAL(121, 1542, 9), // "helpAbout"
QT_MOC_LITERAL(122, 1552, 23), // "helpCheckersInANutshell"
QT_MOC_LITERAL(123, 1576, 12), // "helpHomepage"
QT_MOC_LITERAL(124, 1589, 19), // "helpProblemOfTheDay"
QT_MOC_LITERAL(125, 1609, 17), // "helpOnlineUpgrade"
QT_MOC_LITERAL(126, 1627, 11), // "helpAboutQt"
QT_MOC_LITERAL(127, 1639, 12), // "helpContents"
QT_MOC_LITERAL(128, 1652, 18), // "handleBoardUpdated"
QT_MOC_LITERAL(129, 1671, 8), // "Board8x8"
QT_MOC_LITERAL(130, 1680, 5), // "board"
QT_MOC_LITERAL(131, 1686, 17), // "handleGameMessage"
QT_MOC_LITERAL(132, 1704, 7), // "message"
QT_MOC_LITERAL(133, 1712, 14), // "handleGameOver"
QT_MOC_LITERAL(134, 1727, 6) // "result"

    },
    "MainWindow\0setPrimaryEnginePath\0\0path\0"
    "setSecondaryEnginePath\0changeAppState\0"
    "AppState\0newState\0setStatusBarText\0"
    "text\0updateEvaluationDisplay\0score\0"
    "handleSearchFinished\0moveFound\0aborted\0"
    "CBmove\0bestMove\0statusText\0gameResult\0"
    "pdnMoveText\0elapsedTime\0newGame\0"
    "exitApplication\0game3Move\0gameLoad\0"
    "gameSave\0gameInfo\0gameAnalyze\0gameCopy\0"
    "gamePaste\0gameDatabase\0gameFind\0"
    "gameFindCR\0gameFindTheme\0gameSaveAsHtml\0"
    "gameDiagram\0gameFindPlayer\0"
    "gameFenToClipboard\0gameFenFromClipboard\0"
    "gameSelectUserBook\0gameReSearch\0"
    "gameLoadNext\0gameLoadPrevious\0"
    "gameAnalyzePdn\0gameSampleDiagram\0"
    "movesPlay\0movesBack\0movesForward\0"
    "movesBackAll\0movesForwardAll\0movesComment\0"
    "interruptEngine\0abortEngine\0levelExact\0"
    "levelInstant\0level01S\0level02S\0level05S\0"
    "level1S\0level2S\0level5S\0level10S\0"
    "level15S\0level30S\0level1M\0level2M\0"
    "level5M\0level15M\0level30M\0levelInfinite\0"
    "levelIncrement\0levelAddTime\0"
    "levelSubtractTime\0pieceSet\0optionsHighlight\0"
    "optionsSound\0optionsPriority\0options3Move\0"
    "optionsDirectories\0optionsUserBook\0"
    "optionsLanguageEnglish\0optionsLanguageEspanol\0"
    "optionsLanguageItaliano\0optionsLanguageDeutsch\0"
    "optionsLanguageFrancais\0displayInvert\0"
    "displayNumbers\0displayMirror\0cmNormal\0"
    "cmAnalysis\0gotoNormal\0cmAutoplay\0"
    "cm2Player\0engineVsEngine\0colorBoardNumbers\0"
    "colorHighlight\0bookModeView\0bookModeAdd\0"
    "bookModeDelete\0engineSelect\0engineAbout\0"
    "engineHelp\0engineOptions\0engineAnalyze\0"
    "engineInfinite\0engineResign\0engineDraw\0"
    "engineUndoMove\0enginePonder\0cmEngineMatch\0"
    "cmAddComment\0engineEval\0cmEngineCommand\0"
    "cmRunTestSet\0cmHandicap\0setupMode\0"
    "setupClear\0setupBlack\0setupWhite\0"
    "setupCc\0helpHelp\0helpAbout\0"
    "helpCheckersInANutshell\0helpHomepage\0"
    "helpProblemOfTheDay\0helpOnlineUpgrade\0"
    "helpAboutQt\0helpContents\0handleBoardUpdated\0"
    "Board8x8\0board\0handleGameMessage\0"
    "message\0handleGameOver\0result"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
     116,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  594,    2, 0x06 /* Public */,
       4,    1,  597,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,  600,    2, 0x0a /* Public */,
       8,    1,  603,    2, 0x0a /* Public */,
      10,    1,  606,    2, 0x0a /* Public */,
      12,    7,  609,    2, 0x08 /* Private */,
      21,    0,  624,    2, 0x08 /* Private */,
      22,    0,  625,    2, 0x08 /* Private */,
      23,    0,  626,    2, 0x08 /* Private */,
      24,    0,  627,    2, 0x08 /* Private */,
      25,    0,  628,    2, 0x08 /* Private */,
      26,    0,  629,    2, 0x08 /* Private */,
      27,    0,  630,    2, 0x08 /* Private */,
      28,    0,  631,    2, 0x08 /* Private */,
      29,    0,  632,    2, 0x08 /* Private */,
      30,    0,  633,    2, 0x08 /* Private */,
      31,    0,  634,    2, 0x08 /* Private */,
      32,    0,  635,    2, 0x08 /* Private */,
      33,    0,  636,    2, 0x08 /* Private */,
      34,    0,  637,    2, 0x08 /* Private */,
      35,    0,  638,    2, 0x08 /* Private */,
      36,    0,  639,    2, 0x08 /* Private */,
      37,    0,  640,    2, 0x08 /* Private */,
      38,    0,  641,    2, 0x08 /* Private */,
      39,    0,  642,    2, 0x08 /* Private */,
      40,    0,  643,    2, 0x08 /* Private */,
      41,    0,  644,    2, 0x08 /* Private */,
      42,    0,  645,    2, 0x08 /* Private */,
      43,    0,  646,    2, 0x08 /* Private */,
      44,    0,  647,    2, 0x08 /* Private */,
      45,    0,  648,    2, 0x08 /* Private */,
      46,    0,  649,    2, 0x08 /* Private */,
      47,    0,  650,    2, 0x08 /* Private */,
      48,    0,  651,    2, 0x08 /* Private */,
      49,    0,  652,    2, 0x08 /* Private */,
      50,    0,  653,    2, 0x08 /* Private */,
      51,    0,  654,    2, 0x08 /* Private */,
      52,    0,  655,    2, 0x08 /* Private */,
      53,    0,  656,    2, 0x08 /* Private */,
      54,    0,  657,    2, 0x08 /* Private */,
      55,    0,  658,    2, 0x08 /* Private */,
      56,    0,  659,    2, 0x08 /* Private */,
      57,    0,  660,    2, 0x08 /* Private */,
      58,    0,  661,    2, 0x08 /* Private */,
      59,    0,  662,    2, 0x08 /* Private */,
      60,    0,  663,    2, 0x08 /* Private */,
      61,    0,  664,    2, 0x08 /* Private */,
      62,    0,  665,    2, 0x08 /* Private */,
      63,    0,  666,    2, 0x08 /* Private */,
      64,    0,  667,    2, 0x08 /* Private */,
      65,    0,  668,    2, 0x08 /* Private */,
      66,    0,  669,    2, 0x08 /* Private */,
      67,    0,  670,    2, 0x08 /* Private */,
      68,    0,  671,    2, 0x08 /* Private */,
      69,    0,  672,    2, 0x08 /* Private */,
      70,    0,  673,    2, 0x08 /* Private */,
      71,    0,  674,    2, 0x08 /* Private */,
      72,    0,  675,    2, 0x08 /* Private */,
      73,    0,  676,    2, 0x08 /* Private */,
      74,    0,  677,    2, 0x08 /* Private */,
      75,    0,  678,    2, 0x08 /* Private */,
      76,    0,  679,    2, 0x08 /* Private */,
      77,    0,  680,    2, 0x08 /* Private */,
      78,    0,  681,    2, 0x08 /* Private */,
      79,    0,  682,    2, 0x08 /* Private */,
      80,    0,  683,    2, 0x08 /* Private */,
      81,    0,  684,    2, 0x08 /* Private */,
      82,    0,  685,    2, 0x08 /* Private */,
      83,    0,  686,    2, 0x08 /* Private */,
      84,    0,  687,    2, 0x08 /* Private */,
      85,    0,  688,    2, 0x08 /* Private */,
      86,    0,  689,    2, 0x08 /* Private */,
      87,    0,  690,    2, 0x08 /* Private */,
      88,    0,  691,    2, 0x08 /* Private */,
      89,    0,  692,    2, 0x08 /* Private */,
      90,    0,  693,    2, 0x08 /* Private */,
      91,    0,  694,    2, 0x08 /* Private */,
      92,    0,  695,    2, 0x08 /* Private */,
      93,    0,  696,    2, 0x08 /* Private */,
      94,    0,  697,    2, 0x08 /* Private */,
      95,    0,  698,    2, 0x08 /* Private */,
      96,    0,  699,    2, 0x08 /* Private */,
      97,    0,  700,    2, 0x08 /* Private */,
      98,    0,  701,    2, 0x08 /* Private */,
      99,    0,  702,    2, 0x08 /* Private */,
     100,    0,  703,    2, 0x08 /* Private */,
     101,    0,  704,    2, 0x08 /* Private */,
     102,    0,  705,    2, 0x08 /* Private */,
     103,    0,  706,    2, 0x08 /* Private */,
     104,    0,  707,    2, 0x08 /* Private */,
     105,    0,  708,    2, 0x08 /* Private */,
     106,    0,  709,    2, 0x08 /* Private */,
     107,    0,  710,    2, 0x08 /* Private */,
     108,    0,  711,    2, 0x08 /* Private */,
     109,    0,  712,    2, 0x08 /* Private */,
     110,    0,  713,    2, 0x08 /* Private */,
     111,    0,  714,    2, 0x08 /* Private */,
     112,    0,  715,    2, 0x08 /* Private */,
     113,    0,  716,    2, 0x08 /* Private */,
     114,    0,  717,    2, 0x08 /* Private */,
     115,    0,  718,    2, 0x08 /* Private */,
     116,    0,  719,    2, 0x08 /* Private */,
     117,    0,  720,    2, 0x08 /* Private */,
     118,    0,  721,    2, 0x08 /* Private */,
     119,    0,  722,    2, 0x08 /* Private */,
     120,    0,  723,    2, 0x08 /* Private */,
     121,    0,  724,    2, 0x08 /* Private */,
     122,    0,  725,    2, 0x08 /* Private */,
     123,    0,  726,    2, 0x08 /* Private */,
     124,    0,  727,    2, 0x08 /* Private */,
     125,    0,  728,    2, 0x08 /* Private */,
     126,    0,  729,    2, 0x08 /* Private */,
     127,    0,  730,    2, 0x08 /* Private */,
     128,    1,  731,    2, 0x08 /* Private */,
     131,    1,  734,    2, 0x08 /* Private */,
     133,    1,  737,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void, QMetaType::Bool, QMetaType::Bool, 0x80000000 | 15, QMetaType::QString, QMetaType::Int, QMetaType::QString, QMetaType::Double,   13,   14,   16,   17,   18,   19,   20,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 129,  130,
    QMetaType::Void, QMetaType::QString,  132,
    QMetaType::Void, QMetaType::Int,  134,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->setPrimaryEnginePath((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->setSecondaryEnginePath((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->changeAppState((*reinterpret_cast< AppState(*)>(_a[1]))); break;
        case 3: _t->setStatusBarText((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->updateEvaluationDisplay((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->handleSearchFinished((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< const CBmove(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5])),(*reinterpret_cast< const QString(*)>(_a[6])),(*reinterpret_cast< double(*)>(_a[7]))); break;
        case 6: _t->newGame(); break;
        case 7: _t->exitApplication(); break;
        case 8: _t->game3Move(); break;
        case 9: _t->gameLoad(); break;
        case 10: _t->gameSave(); break;
        case 11: _t->gameInfo(); break;
        case 12: _t->gameAnalyze(); break;
        case 13: _t->gameCopy(); break;
        case 14: _t->gamePaste(); break;
        case 15: _t->gameDatabase(); break;
        case 16: _t->gameFind(); break;
        case 17: _t->gameFindCR(); break;
        case 18: _t->gameFindTheme(); break;
        case 19: _t->gameSaveAsHtml(); break;
        case 20: _t->gameDiagram(); break;
        case 21: _t->gameFindPlayer(); break;
        case 22: _t->gameFenToClipboard(); break;
        case 23: _t->gameFenFromClipboard(); break;
        case 24: _t->gameSelectUserBook(); break;
        case 25: _t->gameReSearch(); break;
        case 26: _t->gameLoadNext(); break;
        case 27: _t->gameLoadPrevious(); break;
        case 28: _t->gameAnalyzePdn(); break;
        case 29: _t->gameSampleDiagram(); break;
        case 30: _t->movesPlay(); break;
        case 31: _t->movesBack(); break;
        case 32: _t->movesForward(); break;
        case 33: _t->movesBackAll(); break;
        case 34: _t->movesForwardAll(); break;
        case 35: _t->movesComment(); break;
        case 36: _t->interruptEngine(); break;
        case 37: _t->abortEngine(); break;
        case 38: _t->levelExact(); break;
        case 39: _t->levelInstant(); break;
        case 40: _t->level01S(); break;
        case 41: _t->level02S(); break;
        case 42: _t->level05S(); break;
        case 43: _t->level1S(); break;
        case 44: _t->level2S(); break;
        case 45: _t->level5S(); break;
        case 46: _t->level10S(); break;
        case 47: _t->level15S(); break;
        case 48: _t->level30S(); break;
        case 49: _t->level1M(); break;
        case 50: _t->level2M(); break;
        case 51: _t->level5M(); break;
        case 52: _t->level15M(); break;
        case 53: _t->level30M(); break;
        case 54: _t->levelInfinite(); break;
        case 55: _t->levelIncrement(); break;
        case 56: _t->levelAddTime(); break;
        case 57: _t->levelSubtractTime(); break;
        case 58: _t->pieceSet(); break;
        case 59: _t->optionsHighlight(); break;
        case 60: _t->optionsSound(); break;
        case 61: _t->optionsPriority(); break;
        case 62: _t->options3Move(); break;
        case 63: _t->optionsDirectories(); break;
        case 64: _t->optionsUserBook(); break;
        case 65: _t->optionsLanguageEnglish(); break;
        case 66: _t->optionsLanguageEspanol(); break;
        case 67: _t->optionsLanguageItaliano(); break;
        case 68: _t->optionsLanguageDeutsch(); break;
        case 69: _t->optionsLanguageFrancais(); break;
        case 70: _t->displayInvert(); break;
        case 71: _t->displayNumbers(); break;
        case 72: _t->displayMirror(); break;
        case 73: _t->cmNormal(); break;
        case 74: _t->cmAnalysis(); break;
        case 75: _t->gotoNormal(); break;
        case 76: _t->cmAutoplay(); break;
        case 77: _t->cm2Player(); break;
        case 78: _t->engineVsEngine(); break;
        case 79: _t->colorBoardNumbers(); break;
        case 80: _t->colorHighlight(); break;
        case 81: _t->bookModeView(); break;
        case 82: _t->bookModeAdd(); break;
        case 83: _t->bookModeDelete(); break;
        case 84: _t->engineSelect(); break;
        case 85: _t->engineAbout(); break;
        case 86: _t->engineHelp(); break;
        case 87: _t->engineOptions(); break;
        case 88: _t->engineAnalyze(); break;
        case 89: _t->engineInfinite(); break;
        case 90: _t->engineResign(); break;
        case 91: _t->engineDraw(); break;
        case 92: _t->engineUndoMove(); break;
        case 93: _t->enginePonder(); break;
        case 94: _t->cmEngineMatch(); break;
        case 95: _t->cmAddComment(); break;
        case 96: _t->engineEval(); break;
        case 97: _t->cmEngineCommand(); break;
        case 98: _t->cmRunTestSet(); break;
        case 99: _t->cmHandicap(); break;
        case 100: _t->setupMode(); break;
        case 101: _t->setupClear(); break;
        case 102: _t->setupBlack(); break;
        case 103: _t->setupWhite(); break;
        case 104: _t->setupCc(); break;
        case 105: _t->helpHelp(); break;
        case 106: _t->helpAbout(); break;
        case 107: _t->helpCheckersInANutshell(); break;
        case 108: _t->helpHomepage(); break;
        case 109: _t->helpProblemOfTheDay(); break;
        case 110: _t->helpOnlineUpgrade(); break;
        case 111: _t->helpAboutQt(); break;
        case 112: _t->helpContents(); break;
        case 113: _t->handleBoardUpdated((*reinterpret_cast< const Board8x8(*)>(_a[1]))); break;
        case 114: _t->handleGameMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 115: _t->handleGameOver((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 2:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< CBmove >(); break;
            }
            break;
        case 113:
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
            using _t = void (MainWindow::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::setPrimaryEnginePath)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::setSecondaryEnginePath)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 116)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 116;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 116)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 116;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::setPrimaryEnginePath(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MainWindow::setSecondaryEnginePath(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
