/****************************************************************************
** Meta object code from reading C++ file 'engine_wrapper.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "engine_wrapper.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'engine_wrapper.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ExternalEngine_t {
    QByteArrayData data[23];
    char stringdata0[297];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ExternalEngine_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ExternalEngine_t qt_meta_stringdata_ExternalEngine = {
    {
QT_MOC_LITERAL(0, 0, 14), // "ExternalEngine"
QT_MOC_LITERAL(1, 15, 12), // "engineOutput"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 6), // "output"
QT_MOC_LITERAL(4, 36, 11), // "engineError"
QT_MOC_LITERAL(5, 48, 5), // "error"
QT_MOC_LITERAL(6, 54, 11), // "engineReady"
QT_MOC_LITERAL(7, 66, 14), // "engineResponse"
QT_MOC_LITERAL(8, 81, 8), // "response"
QT_MOC_LITERAL(9, 90, 13), // "bestMoveFound"
QT_MOC_LITERAL(10, 104, 10), // "moveString"
QT_MOC_LITERAL(11, 115, 16), // "evaluationUpdate"
QT_MOC_LITERAL(12, 132, 5), // "score"
QT_MOC_LITERAL(13, 138, 5), // "depth"
QT_MOC_LITERAL(14, 144, 18), // "readStandardOutput"
QT_MOC_LITERAL(15, 163, 17), // "readStandardError"
QT_MOC_LITERAL(16, 181, 14), // "processStarted"
QT_MOC_LITERAL(17, 196, 15), // "processFinished"
QT_MOC_LITERAL(18, 212, 8), // "exitCode"
QT_MOC_LITERAL(19, 221, 20), // "QProcess::ExitStatus"
QT_MOC_LITERAL(20, 242, 10), // "exitStatus"
QT_MOC_LITERAL(21, 253, 20), // "processErrorOccurred"
QT_MOC_LITERAL(22, 274, 22) // "QProcess::ProcessError"

    },
    "ExternalEngine\0engineOutput\0\0output\0"
    "engineError\0error\0engineReady\0"
    "engineResponse\0response\0bestMoveFound\0"
    "moveString\0evaluationUpdate\0score\0"
    "depth\0readStandardOutput\0readStandardError\0"
    "processStarted\0processFinished\0exitCode\0"
    "QProcess::ExitStatus\0exitStatus\0"
    "processErrorOccurred\0QProcess::ProcessError"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ExternalEngine[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       4,    1,   72,    2, 0x06 /* Public */,
       6,    0,   75,    2, 0x06 /* Public */,
       7,    1,   76,    2, 0x06 /* Public */,
       9,    1,   79,    2, 0x06 /* Public */,
      11,    2,   82,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      14,    0,   87,    2, 0x08 /* Private */,
      15,    0,   88,    2, 0x08 /* Private */,
      16,    0,   89,    2, 0x08 /* Private */,
      17,    2,   90,    2, 0x08 /* Private */,
      21,    1,   95,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   12,   13,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 19,   18,   20,
    QMetaType::Void, 0x80000000 | 22,    5,

       0        // eod
};

void ExternalEngine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ExternalEngine *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->engineOutput((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->engineError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->engineReady(); break;
        case 3: _t->engineResponse((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->bestMoveFound((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->evaluationUpdate((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->readStandardOutput(); break;
        case 7: _t->readStandardError(); break;
        case 8: _t->processStarted(); break;
        case 9: _t->processFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        case 10: _t->processErrorOccurred((*reinterpret_cast< QProcess::ProcessError(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ExternalEngine::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExternalEngine::engineOutput)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ExternalEngine::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExternalEngine::engineError)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ExternalEngine::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExternalEngine::engineReady)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ExternalEngine::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExternalEngine::engineResponse)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ExternalEngine::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExternalEngine::bestMoveFound)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (ExternalEngine::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExternalEngine::evaluationUpdate)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ExternalEngine::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ExternalEngine.data,
    qt_meta_data_ExternalEngine,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ExternalEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ExternalEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ExternalEngine.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ExternalEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void ExternalEngine::engineOutput(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ExternalEngine::engineError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ExternalEngine::engineReady()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ExternalEngine::engineResponse(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void ExternalEngine::bestMoveFound(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void ExternalEngine::evaluationUpdate(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
