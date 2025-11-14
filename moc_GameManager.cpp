/****************************************************************************
** Meta object code from reading C++ file 'GameManager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "GameManager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GameManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GameManager_t {
    QByteArrayData data[25];
    char stringdata0[276];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GameManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GameManager_t qt_meta_stringdata_GameManager = {
    {
QT_MOC_LITERAL(0, 0, 11), // "GameManager"
QT_MOC_LITERAL(1, 12, 12), // "boardUpdated"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 8), // "Board8x8"
QT_MOC_LITERAL(4, 35, 5), // "board"
QT_MOC_LITERAL(5, 41, 11), // "gameMessage"
QT_MOC_LITERAL(6, 53, 7), // "message"
QT_MOC_LITERAL(7, 61, 10), // "gameIsOver"
QT_MOC_LITERAL(8, 72, 6), // "result"
QT_MOC_LITERAL(9, 79, 19), // "requestEngineSearch"
QT_MOC_LITERAL(10, 99, 11), // "colorToMove"
QT_MOC_LITERAL(11, 111, 9), // "timeLimit"
QT_MOC_LITERAL(12, 121, 13), // "pieceSelected"
QT_MOC_LITERAL(13, 135, 1), // "x"
QT_MOC_LITERAL(14, 137, 1), // "y"
QT_MOC_LITERAL(15, 139, 15), // "pieceDeselected"
QT_MOC_LITERAL(16, 155, 9), // "humanTurn"
QT_MOC_LITERAL(17, 165, 17), // "evaluationUpdated"
QT_MOC_LITERAL(18, 183, 5), // "score"
QT_MOC_LITERAL(19, 189, 5), // "depth"
QT_MOC_LITERAL(20, 195, 18), // "updateClockDisplay"
QT_MOC_LITERAL(21, 214, 9), // "whiteTime"
QT_MOC_LITERAL(22, 224, 9), // "blackTime"
QT_MOC_LITERAL(23, 234, 22), // "handleEvaluationUpdate"
QT_MOC_LITERAL(24, 257, 18) // "handleTimerTimeout"

    },
    "GameManager\0boardUpdated\0\0Board8x8\0"
    "board\0gameMessage\0message\0gameIsOver\0"
    "result\0requestEngineSearch\0colorToMove\0"
    "timeLimit\0pieceSelected\0x\0y\0pieceDeselected\0"
    "humanTurn\0evaluationUpdated\0score\0"
    "depth\0updateClockDisplay\0whiteTime\0"
    "blackTime\0handleEvaluationUpdate\0"
    "handleTimerTimeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GameManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       5,    1,   72,    2, 0x06 /* Public */,
       7,    1,   75,    2, 0x06 /* Public */,
       9,    3,   78,    2, 0x06 /* Public */,
      12,    2,   85,    2, 0x06 /* Public */,
      15,    0,   90,    2, 0x06 /* Public */,
      16,    0,   91,    2, 0x06 /* Public */,
      17,    2,   92,    2, 0x06 /* Public */,
      20,    2,   97,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      23,    2,  102,    2, 0x0a /* Public */,
      24,    0,  107,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int, QMetaType::Double,    4,   10,   11,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   13,   14,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   18,   19,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,   21,   22,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   18,   19,
    QMetaType::Void,

       0        // eod
};

void GameManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GameManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->boardUpdated((*reinterpret_cast< const Board8x8(*)>(_a[1]))); break;
        case 1: _t->gameMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->gameIsOver((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->requestEngineSearch((*reinterpret_cast< const Board8x8(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 4: _t->pieceSelected((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->pieceDeselected(); break;
        case 6: _t->humanTurn(); break;
        case 7: _t->evaluationUpdated((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: _t->updateClockDisplay((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 9: _t->handleEvaluationUpdate((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 10: _t->handleTimerTimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< Board8x8 >(); break;
            }
            break;
        case 3:
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
            using _t = void (GameManager::*)(const Board8x8 & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameManager::boardUpdated)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GameManager::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameManager::gameMessage)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (GameManager::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameManager::gameIsOver)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (GameManager::*)(const Board8x8 & , int , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameManager::requestEngineSearch)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (GameManager::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameManager::pieceSelected)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (GameManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameManager::pieceDeselected)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (GameManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameManager::humanTurn)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (GameManager::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameManager::evaluationUpdated)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (GameManager::*)(double , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameManager::updateClockDisplay)) {
                *result = 8;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject GameManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_GameManager.data,
    qt_meta_data_GameManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *GameManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GameManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GameManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GameManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void GameManager::boardUpdated(const Board8x8 & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void GameManager::gameMessage(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void GameManager::gameIsOver(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void GameManager::requestEngineSearch(const Board8x8 & _t1, int _t2, double _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void GameManager::pieceSelected(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void GameManager::pieceDeselected()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void GameManager::humanTurn()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void GameManager::evaluationUpdated(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void GameManager::updateClockDisplay(double _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
