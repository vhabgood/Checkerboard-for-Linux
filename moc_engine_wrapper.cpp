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
    QByteArrayData data[22];
    char stringdata0[274];
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
QT_MOC_LITERAL(7, 66, 13), // "bestMoveFound"
QT_MOC_LITERAL(8, 80, 6), // "CBmove"
QT_MOC_LITERAL(9, 87, 4), // "move"
QT_MOC_LITERAL(10, 92, 16), // "evaluationUpdate"
QT_MOC_LITERAL(11, 109, 5), // "score"
QT_MOC_LITERAL(12, 115, 5), // "depth"
QT_MOC_LITERAL(13, 121, 18), // "readStandardOutput"
QT_MOC_LITERAL(14, 140, 17), // "readStandardError"
QT_MOC_LITERAL(15, 158, 14), // "processStarted"
QT_MOC_LITERAL(16, 173, 15), // "processFinished"
QT_MOC_LITERAL(17, 189, 8), // "exitCode"
QT_MOC_LITERAL(18, 198, 20), // "QProcess::ExitStatus"
QT_MOC_LITERAL(19, 219, 10), // "exitStatus"
QT_MOC_LITERAL(20, 230, 20), // "processErrorOccurred"
QT_MOC_LITERAL(21, 251, 22) // "QProcess::ProcessError"

    },
    "ExternalEngine\0engineOutput\0\0output\0"
    "engineError\0error\0engineReady\0"
    "bestMoveFound\0CBmove\0move\0evaluationUpdate\0"
    "score\0depth\0readStandardOutput\0"
    "readStandardError\0processStarted\0"
    "processFinished\0exitCode\0QProcess::ExitStatus\0"
    "exitStatus\0processErrorOccurred\0"
    "QProcess::ProcessError"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ExternalEngine[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   64,    2, 0x06 /* Public */,
       4,    1,   67,    2, 0x06 /* Public */,
       6,    0,   70,    2, 0x06 /* Public */,
       7,    1,   71,    2, 0x06 /* Public */,
      10,    2,   74,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    0,   79,    2, 0x08 /* Private */,
      14,    0,   80,    2, 0x08 /* Private */,
      15,    0,   81,    2, 0x08 /* Private */,
      16,    2,   82,    2, 0x08 /* Private */,
      20,    1,   87,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   11,   12,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 18,   17,   19,
    QMetaType::Void, 0x80000000 | 21,    5,

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
        case 3: _t->bestMoveFound((*reinterpret_cast< const CBmove(*)>(_a[1]))); break;
        case 4: _t->evaluationUpdate((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->readStandardOutput(); break;
        case 6: _t->readStandardError(); break;
        case 7: _t->processStarted(); break;
        case 8: _t->processFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        case 9: _t->processErrorOccurred((*reinterpret_cast< QProcess::ProcessError(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< CBmove >(); break;
            }
            break;
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
            using _t = void (ExternalEngine::*)(const CBmove & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExternalEngine::bestMoveFound)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ExternalEngine::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ExternalEngine::evaluationUpdate)) {
                *result = 4;
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
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
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
void ExternalEngine::bestMoveFound(const CBmove & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void ExternalEngine::evaluationUpdate(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
