/****************************************************************************
** Meta object code from reading C++ file 'GeminiAI.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "GeminiAI.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GeminiAI.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GeminiAI_t {
    QByteArrayData data[42];
    char stringdata0[495];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GeminiAI_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GeminiAI_t qt_meta_stringdata_GeminiAI = {
    {
QT_MOC_LITERAL(0, 0, 8), // "GeminiAI"
QT_MOC_LITERAL(1, 9, 14), // "searchFinished"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 9), // "moveFound"
QT_MOC_LITERAL(4, 35, 7), // "aborted"
QT_MOC_LITERAL(5, 43, 6), // "CBmove"
QT_MOC_LITERAL(6, 50, 8), // "bestMove"
QT_MOC_LITERAL(7, 59, 10), // "statusText"
QT_MOC_LITERAL(8, 70, 10), // "gameResult"
QT_MOC_LITERAL(9, 81, 11), // "pdnMoveText"
QT_MOC_LITERAL(10, 93, 11), // "elapsedTime"
QT_MOC_LITERAL(11, 105, 11), // "engineError"
QT_MOC_LITERAL(12, 117, 12), // "errorMessage"
QT_MOC_LITERAL(13, 130, 14), // "requestNewGame"
QT_MOC_LITERAL(14, 145, 8), // "gameType"
QT_MOC_LITERAL(15, 154, 12), // "updateStatus"
QT_MOC_LITERAL(16, 167, 13), // "statusMessage"
QT_MOC_LITERAL(17, 181, 11), // "changeState"
QT_MOC_LITERAL(18, 193, 8), // "AppState"
QT_MOC_LITERAL(19, 202, 8), // "newState"
QT_MOC_LITERAL(20, 211, 15), // "evaluationReady"
QT_MOC_LITERAL(21, 227, 5), // "score"
QT_MOC_LITERAL(22, 233, 5), // "depth"
QT_MOC_LITERAL(23, 239, 4), // "init"
QT_MOC_LITERAL(24, 244, 11), // "requestMove"
QT_MOC_LITERAL(25, 256, 8), // "Board8x8"
QT_MOC_LITERAL(26, 265, 5), // "board"
QT_MOC_LITERAL(27, 271, 11), // "colorToMove"
QT_MOC_LITERAL(28, 283, 9), // "timeLimit"
QT_MOC_LITERAL(29, 293, 6), // "doWork"
QT_MOC_LITERAL(30, 300, 17), // "initEngineProcess"
QT_MOC_LITERAL(31, 318, 17), // "quitEngineProcess"
QT_MOC_LITERAL(32, 336, 16), // "startAnalyzeGame"
QT_MOC_LITERAL(33, 353, 13), // "startAutoplay"
QT_MOC_LITERAL(34, 367, 16), // "startEngineMatch"
QT_MOC_LITERAL(35, 384, 8), // "numGames"
QT_MOC_LITERAL(36, 393, 15), // "startRunTestSet"
QT_MOC_LITERAL(37, 409, 15), // "startAnalyzePdn"
QT_MOC_LITERAL(38, 425, 11), // "abortSearch"
QT_MOC_LITERAL(39, 437, 21), // "setExternalEnginePath"
QT_MOC_LITERAL(40, 459, 4), // "path"
QT_MOC_LITERAL(41, 464, 30) // "setSecondaryExternalEnginePath"

    },
    "GeminiAI\0searchFinished\0\0moveFound\0"
    "aborted\0CBmove\0bestMove\0statusText\0"
    "gameResult\0pdnMoveText\0elapsedTime\0"
    "engineError\0errorMessage\0requestNewGame\0"
    "gameType\0updateStatus\0statusMessage\0"
    "changeState\0AppState\0newState\0"
    "evaluationReady\0score\0depth\0init\0"
    "requestMove\0Board8x8\0board\0colorToMove\0"
    "timeLimit\0doWork\0initEngineProcess\0"
    "quitEngineProcess\0startAnalyzeGame\0"
    "startAutoplay\0startEngineMatch\0numGames\0"
    "startRunTestSet\0startAnalyzePdn\0"
    "abortSearch\0setExternalEnginePath\0"
    "path\0setSecondaryExternalEnginePath"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GeminiAI[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    7,  109,    2, 0x06 /* Public */,
      11,    1,  124,    2, 0x06 /* Public */,
      13,    1,  127,    2, 0x06 /* Public */,
      15,    1,  130,    2, 0x06 /* Public */,
      17,    1,  133,    2, 0x06 /* Public */,
      20,    2,  136,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      23,    0,  141,    2, 0x0a /* Public */,
      24,    3,  142,    2, 0x0a /* Public */,
      29,    0,  149,    2, 0x0a /* Public */,
      30,    0,  150,    2, 0x0a /* Public */,
      31,    0,  151,    2, 0x0a /* Public */,
      32,    2,  152,    2, 0x0a /* Public */,
      33,    2,  157,    2, 0x0a /* Public */,
      34,    3,  162,    2, 0x0a /* Public */,
      36,    2,  169,    2, 0x0a /* Public */,
      37,    2,  174,    2, 0x0a /* Public */,
      38,    0,  179,    2, 0x0a /* Public */,
      39,    1,  180,    2, 0x0a /* Public */,
      41,    1,  183,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool, QMetaType::Bool, 0x80000000 | 5, QMetaType::QString, QMetaType::Int, QMetaType::QString, QMetaType::Double,    3,    4,    6,    7,    8,    9,   10,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::QString,   16,
    QMetaType::Void, 0x80000000 | 18,   19,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   21,   22,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 25, QMetaType::Int, QMetaType::Double,   26,   27,   28,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 25, QMetaType::Int,   26,   27,
    QMetaType::Void, 0x80000000 | 25, QMetaType::Int,   26,   27,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 25, QMetaType::Int,   35,   26,   27,
    QMetaType::Void, 0x80000000 | 25, QMetaType::Int,   26,   27,
    QMetaType::Void, 0x80000000 | 25, QMetaType::Int,   26,   27,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   40,
    QMetaType::Void, QMetaType::QString,   40,

       0        // eod
};

void GeminiAI::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GeminiAI *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->searchFinished((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< const CBmove(*)>(_a[3])),(*reinterpret_cast< const QString(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5])),(*reinterpret_cast< const QString(*)>(_a[6])),(*reinterpret_cast< double(*)>(_a[7]))); break;
        case 1: _t->engineError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->requestNewGame((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->updateStatus((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->changeState((*reinterpret_cast< AppState(*)>(_a[1]))); break;
        case 5: _t->evaluationReady((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->init(); break;
        case 7: _t->requestMove((*reinterpret_cast< Board8x8(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 8: _t->doWork(); break;
        case 9: _t->initEngineProcess(); break;
        case 10: _t->quitEngineProcess(); break;
        case 11: _t->startAnalyzeGame((*reinterpret_cast< const Board8x8(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 12: _t->startAutoplay((*reinterpret_cast< const Board8x8(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 13: _t->startEngineMatch((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const Board8x8(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 14: _t->startRunTestSet((*reinterpret_cast< const Board8x8(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 15: _t->startAnalyzePdn((*reinterpret_cast< const Board8x8(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 16: _t->abortSearch(); break;
        case 17: _t->setExternalEnginePath((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 18: _t->setSecondaryExternalEnginePath((*reinterpret_cast< const QString(*)>(_a[1]))); break;
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
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Board8x8 >(); break;
            }
            break;
        case 11:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Board8x8 >(); break;
            }
            break;
        case 12:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Board8x8 >(); break;
            }
            break;
        case 13:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 1:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Board8x8 >(); break;
            }
            break;
        case 14:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Board8x8 >(); break;
            }
            break;
        case 15:
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
            using _t = void (GeminiAI::*)(bool , bool , const CBmove & , const QString & , int , const QString & , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GeminiAI::searchFinished)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GeminiAI::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GeminiAI::engineError)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (GeminiAI::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GeminiAI::requestNewGame)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (GeminiAI::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GeminiAI::updateStatus)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (GeminiAI::*)(AppState );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GeminiAI::changeState)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (GeminiAI::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GeminiAI::evaluationReady)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject GeminiAI::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_GeminiAI.data,
    qt_meta_data_GeminiAI,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *GeminiAI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GeminiAI::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GeminiAI.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GeminiAI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void GeminiAI::searchFinished(bool _t1, bool _t2, const CBmove & _t3, const QString & _t4, int _t5, const QString & _t6, double _t7)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t6))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t7))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void GeminiAI::engineError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void GeminiAI::requestNewGame(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void GeminiAI::updateStatus(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void GeminiAI::changeState(AppState _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void GeminiAI::evaluationReady(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
