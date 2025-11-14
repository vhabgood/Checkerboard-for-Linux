/****************************************************************************
** Meta object code from reading C++ file 'test_game_logic.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "test_game_logic.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'test_game_logic.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TestGameLogic_t {
    QByteArrayData data[15];
    char stringdata0[249];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TestGameLogic_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TestGameLogic_t qt_meta_stringdata_TestGameLogic = {
    {
QT_MOC_LITERAL(0, 0, 13), // "TestGameLogic"
QT_MOC_LITERAL(1, 14, 12), // "initTestCase"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 15), // "cleanupTestCase"
QT_MOC_LITERAL(4, 44, 4), // "init"
QT_MOC_LITERAL(5, 49, 7), // "cleanup"
QT_MOC_LITERAL(6, 57, 16), // "testNewGame_data"
QT_MOC_LITERAL(7, 74, 11), // "testNewGame"
QT_MOC_LITERAL(8, 86, 12), // "testMakeMove"
QT_MOC_LITERAL(9, 99, 19), // "testMakeCaptureMove"
QT_MOC_LITERAL(10, 119, 16), // "testInvalidMoves"
QT_MOC_LITERAL(11, 136, 20), // "testGoBackAndForward"
QT_MOC_LITERAL(12, 157, 28), // "testDrawDetection_50MoveRule"
QT_MOC_LITERAL(13, 186, 37), // "testDrawDetection_ThreeFoldRe..."
QT_MOC_LITERAL(14, 224, 24) // "testWinLoss_NoLegalMoves"

    },
    "TestGameLogic\0initTestCase\0\0cleanupTestCase\0"
    "init\0cleanup\0testNewGame_data\0testNewGame\0"
    "testMakeMove\0testMakeCaptureMove\0"
    "testInvalidMoves\0testGoBackAndForward\0"
    "testDrawDetection_50MoveRule\0"
    "testDrawDetection_ThreeFoldRepetition\0"
    "testWinLoss_NoLegalMoves"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TestGameLogic[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x08 /* Private */,
       3,    0,   80,    2, 0x08 /* Private */,
       4,    0,   81,    2, 0x08 /* Private */,
       5,    0,   82,    2, 0x08 /* Private */,
       6,    0,   83,    2, 0x08 /* Private */,
       7,    0,   84,    2, 0x08 /* Private */,
       8,    0,   85,    2, 0x08 /* Private */,
       9,    0,   86,    2, 0x08 /* Private */,
      10,    0,   87,    2, 0x08 /* Private */,
      11,    0,   88,    2, 0x08 /* Private */,
      12,    0,   89,    2, 0x08 /* Private */,
      13,    0,   90,    2, 0x08 /* Private */,
      14,    0,   91,    2, 0x08 /* Private */,

 // slots: parameters
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

       0        // eod
};

void TestGameLogic::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TestGameLogic *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->initTestCase(); break;
        case 1: _t->cleanupTestCase(); break;
        case 2: _t->init(); break;
        case 3: _t->cleanup(); break;
        case 4: _t->testNewGame_data(); break;
        case 5: _t->testNewGame(); break;
        case 6: _t->testMakeMove(); break;
        case 7: _t->testMakeCaptureMove(); break;
        case 8: _t->testInvalidMoves(); break;
        case 9: _t->testGoBackAndForward(); break;
        case 10: _t->testDrawDetection_50MoveRule(); break;
        case 11: _t->testDrawDetection_ThreeFoldRepetition(); break;
        case 12: _t->testWinLoss_NoLegalMoves(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject TestGameLogic::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_TestGameLogic.data,
    qt_meta_data_TestGameLogic,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TestGameLogic::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TestGameLogic::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TestGameLogic.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TestGameLogic::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
