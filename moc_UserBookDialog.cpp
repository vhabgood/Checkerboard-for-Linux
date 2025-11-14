/****************************************************************************
** Meta object code from reading C++ file 'UserBookDialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "UserBookDialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UserBookDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_UserBookDialog_t {
    QByteArrayData data[19];
    char stringdata0[401];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_UserBookDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_UserBookDialog_t qt_meta_stringdata_UserBookDialog = {
    {
QT_MOC_LITERAL(0, 0, 14), // "UserBookDialog"
QT_MOC_LITERAL(1, 15, 21), // "loadUserBookRequested"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 8), // "filename"
QT_MOC_LITERAL(4, 47, 21), // "saveUserBookRequested"
QT_MOC_LITERAL(5, 69, 26), // "addMoveToUserBookRequested"
QT_MOC_LITERAL(6, 96, 27), // "deleteCurrentEntryRequested"
QT_MOC_LITERAL(7, 124, 28), // "navigateToNextEntryRequested"
QT_MOC_LITERAL(8, 153, 32), // "navigateToPreviousEntryRequested"
QT_MOC_LITERAL(9, 186, 24), // "resetNavigationRequested"
QT_MOC_LITERAL(10, 211, 19), // "onLoadButtonClicked"
QT_MOC_LITERAL(11, 231, 19), // "onSaveButtonClicked"
QT_MOC_LITERAL(12, 251, 18), // "onAddButtonClicked"
QT_MOC_LITERAL(13, 270, 21), // "onDeleteButtonClicked"
QT_MOC_LITERAL(14, 292, 19), // "onNextButtonClicked"
QT_MOC_LITERAL(15, 312, 23), // "onPreviousButtonClicked"
QT_MOC_LITERAL(16, 336, 20), // "onResetButtonClicked"
QT_MOC_LITERAL(17, 357, 21), // "on_buttonBox_accepted"
QT_MOC_LITERAL(18, 379, 21) // "on_buttonBox_rejected"

    },
    "UserBookDialog\0loadUserBookRequested\0"
    "\0filename\0saveUserBookRequested\0"
    "addMoveToUserBookRequested\0"
    "deleteCurrentEntryRequested\0"
    "navigateToNextEntryRequested\0"
    "navigateToPreviousEntryRequested\0"
    "resetNavigationRequested\0onLoadButtonClicked\0"
    "onSaveButtonClicked\0onAddButtonClicked\0"
    "onDeleteButtonClicked\0onNextButtonClicked\0"
    "onPreviousButtonClicked\0onResetButtonClicked\0"
    "on_buttonBox_accepted\0on_buttonBox_rejected"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_UserBookDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   94,    2, 0x06 /* Public */,
       4,    1,   97,    2, 0x06 /* Public */,
       5,    0,  100,    2, 0x06 /* Public */,
       6,    0,  101,    2, 0x06 /* Public */,
       7,    0,  102,    2, 0x06 /* Public */,
       8,    0,  103,    2, 0x06 /* Public */,
       9,    0,  104,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,  105,    2, 0x08 /* Private */,
      11,    0,  106,    2, 0x08 /* Private */,
      12,    0,  107,    2, 0x08 /* Private */,
      13,    0,  108,    2, 0x08 /* Private */,
      14,    0,  109,    2, 0x08 /* Private */,
      15,    0,  110,    2, 0x08 /* Private */,
      16,    0,  111,    2, 0x08 /* Private */,
      17,    0,  112,    2, 0x08 /* Private */,
      18,    0,  113,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

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

       0        // eod
};

void UserBookDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<UserBookDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->loadUserBookRequested((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->saveUserBookRequested((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->addMoveToUserBookRequested(); break;
        case 3: _t->deleteCurrentEntryRequested(); break;
        case 4: _t->navigateToNextEntryRequested(); break;
        case 5: _t->navigateToPreviousEntryRequested(); break;
        case 6: _t->resetNavigationRequested(); break;
        case 7: _t->onLoadButtonClicked(); break;
        case 8: _t->onSaveButtonClicked(); break;
        case 9: _t->onAddButtonClicked(); break;
        case 10: _t->onDeleteButtonClicked(); break;
        case 11: _t->onNextButtonClicked(); break;
        case 12: _t->onPreviousButtonClicked(); break;
        case 13: _t->onResetButtonClicked(); break;
        case 14: _t->on_buttonBox_accepted(); break;
        case 15: _t->on_buttonBox_rejected(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (UserBookDialog::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UserBookDialog::loadUserBookRequested)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (UserBookDialog::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UserBookDialog::saveUserBookRequested)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (UserBookDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UserBookDialog::addMoveToUserBookRequested)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (UserBookDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UserBookDialog::deleteCurrentEntryRequested)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (UserBookDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UserBookDialog::navigateToNextEntryRequested)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (UserBookDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UserBookDialog::navigateToPreviousEntryRequested)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (UserBookDialog::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UserBookDialog::resetNavigationRequested)) {
                *result = 6;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject UserBookDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_UserBookDialog.data,
    qt_meta_data_UserBookDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *UserBookDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UserBookDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_UserBookDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int UserBookDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void UserBookDialog::loadUserBookRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void UserBookDialog::saveUserBookRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void UserBookDialog::addMoveToUserBookRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void UserBookDialog::deleteCurrentEntryRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void UserBookDialog::navigateToNextEntryRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void UserBookDialog::navigateToPreviousEntryRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void UserBookDialog::resetNavigationRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
