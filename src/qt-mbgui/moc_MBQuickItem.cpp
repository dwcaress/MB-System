/****************************************************************************
** Meta object code from reading C++ file 'MBQuickItem.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "MBQuickItem.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MBQuickItem.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MBQuickItem_t {
    QByteArrayData data[10];
    char stringdata0[120];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MBQuickItem_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MBQuickItem_t qt_meta_stringdata_MBQuickItem = {
    {
QT_MOC_LITERAL(0, 0, 11), // "MBQuickItem"
QT_MOC_LITERAL(1, 12, 19), // "synchronizeUnderlay"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 7), // "cleanup"
QT_MOC_LITERAL(4, 41, 19), // "handleWindowChanged"
QT_MOC_LITERAL(5, 61, 13), // "QQuickWindow*"
QT_MOC_LITERAL(6, 75, 6), // "window"
QT_MOC_LITERAL(7, 82, 14), // "renderUnderlay"
QT_MOC_LITERAL(8, 97, 14), // "setGridSurface"
QT_MOC_LITERAL(9, 112, 7) // "fileURL"

    },
    "MBQuickItem\0synchronizeUnderlay\0\0"
    "cleanup\0handleWindowChanged\0QQuickWindow*\0"
    "window\0renderUnderlay\0setGridSurface\0"
    "fileURL"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MBQuickItem[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x0a /* Public */,
       3,    0,   40,    2, 0x0a /* Public */,
       4,    1,   41,    2, 0x09 /* Protected */,
       7,    0,   44,    2, 0x09 /* Protected */,

 // methods: name, argc, parameters, tag, flags
       8,    1,   45,    2, 0x02 /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Bool, QMetaType::QUrl,    9,

       0        // eod
};

void MBQuickItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MBQuickItem *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->synchronizeUnderlay(); break;
        case 1: _t->cleanup(); break;
        case 2: _t->handleWindowChanged((*reinterpret_cast< QQuickWindow*(*)>(_a[1]))); break;
        case 3: _t->renderUnderlay(); break;
        case 4: { bool _r = _t->setGridSurface((*reinterpret_cast< QUrl(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MBQuickItem::staticMetaObject = { {
    QMetaObject::SuperData::link<QQuickItem::staticMetaObject>(),
    qt_meta_stringdata_MBQuickItem.data,
    qt_meta_data_MBQuickItem,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MBQuickItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MBQuickItem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MBQuickItem.stringdata0))
        return static_cast<void*>(this);
    return QQuickItem::qt_metacast(_clname);
}

int MBQuickItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
