/****************************************************************************
** Meta object code from reading C++ file 'QVTKFramebufferObjectItem.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "QVTKFramebufferObjectItem.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QVTKFramebufferObjectItem.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QVTKFramebufferObjectItem_t {
    QByteArrayData data[9];
    char stringdata0[179];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QVTKFramebufferObjectItem_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QVTKFramebufferObjectItem_t qt_meta_stringdata_QVTKFramebufferObjectItem = {
    {
QT_MOC_LITERAL(0, 0, 25), // "QVTKFramebufferObjectItem"
QT_MOC_LITERAL(1, 26, 19), // "rendererInitialized"
QT_MOC_LITERAL(2, 46, 0), // ""
QT_MOC_LITERAL(3, 47, 22), // "isModelSelectedChanged"
QT_MOC_LITERAL(4, 70, 29), // "selectedModelPositionXChanged"
QT_MOC_LITERAL(5, 100, 29), // "selectedModelPositionYChanged"
QT_MOC_LITERAL(6, 130, 20), // "addModelFromFileDone"
QT_MOC_LITERAL(7, 151, 21), // "addModelFromFileError"
QT_MOC_LITERAL(8, 173, 5) // "error"

    },
    "QVTKFramebufferObjectItem\0rendererInitialized\0"
    "\0isModelSelectedChanged\0"
    "selectedModelPositionXChanged\0"
    "selectedModelPositionYChanged\0"
    "addModelFromFileDone\0addModelFromFileError\0"
    "error"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QVTKFramebufferObjectItem[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,
       3,    0,   45,    2, 0x06 /* Public */,
       4,    0,   46,    2, 0x06 /* Public */,
       5,    0,   47,    2, 0x06 /* Public */,
       6,    0,   48,    2, 0x06 /* Public */,
       7,    1,   49,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,

       0        // eod
};

void QVTKFramebufferObjectItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QVTKFramebufferObjectItem *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->rendererInitialized(); break;
        case 1: _t->isModelSelectedChanged(); break;
        case 2: _t->selectedModelPositionXChanged(); break;
        case 3: _t->selectedModelPositionYChanged(); break;
        case 4: _t->addModelFromFileDone(); break;
        case 5: _t->addModelFromFileError((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (QVTKFramebufferObjectItem::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QVTKFramebufferObjectItem::rendererInitialized)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (QVTKFramebufferObjectItem::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QVTKFramebufferObjectItem::isModelSelectedChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (QVTKFramebufferObjectItem::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QVTKFramebufferObjectItem::selectedModelPositionXChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (QVTKFramebufferObjectItem::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QVTKFramebufferObjectItem::selectedModelPositionYChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (QVTKFramebufferObjectItem::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QVTKFramebufferObjectItem::addModelFromFileDone)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (QVTKFramebufferObjectItem::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QVTKFramebufferObjectItem::addModelFromFileError)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject QVTKFramebufferObjectItem::staticMetaObject = { {
    QMetaObject::SuperData::link<QQuickFramebufferObject::staticMetaObject>(),
    qt_meta_stringdata_QVTKFramebufferObjectItem.data,
    qt_meta_data_QVTKFramebufferObjectItem,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QVTKFramebufferObjectItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QVTKFramebufferObjectItem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QVTKFramebufferObjectItem.stringdata0))
        return static_cast<void*>(this);
    return QQuickFramebufferObject::qt_metacast(_clname);
}

int QVTKFramebufferObjectItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickFramebufferObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void QVTKFramebufferObjectItem::rendererInitialized()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void QVTKFramebufferObjectItem::isModelSelectedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void QVTKFramebufferObjectItem::selectedModelPositionXChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void QVTKFramebufferObjectItem::selectedModelPositionYChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void QVTKFramebufferObjectItem::addModelFromFileDone()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void QVTKFramebufferObjectItem::addModelFromFileError(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
