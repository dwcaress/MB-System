/****************************************************************************
** Meta object code from reading C++ file 'declarativescene_p.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "datavisualizationqml2/declarativescene_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'declarativescene_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtDataVisualization__Declarative3DScene_t {
    QByteArrayData data[6];
    char stringdata0[125];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtDataVisualization__Declarative3DScene_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtDataVisualization__Declarative3DScene_t qt_meta_stringdata_QtDataVisualization__Declarative3DScene = {
    {
QT_MOC_LITERAL(0, 0, 39), // "QtDataVisualization::Declarat..."
QT_MOC_LITERAL(1, 40, 29), // "selectionQueryPositionChanged"
QT_MOC_LITERAL(2, 70, 0), // ""
QT_MOC_LITERAL(3, 71, 8), // "position"
QT_MOC_LITERAL(4, 80, 22), // "selectionQueryPosition"
QT_MOC_LITERAL(5, 103, 21) // "invalidSelectionPoint"

    },
    "QtDataVisualization::Declarative3DScene\0"
    "selectionQueryPositionChanged\0\0position\0"
    "selectionQueryPosition\0invalidSelectionPoint"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtDataVisualization__Declarative3DScene[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       2,   22, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QPointF,    3,

 // properties: name, type, flags
       4, QMetaType::QPointF, 0x00495103,
       5, QMetaType::QPoint, 0x00095401,

 // properties: notify_signal_id
       0,
       0,

       0        // eod
};

void QtDataVisualization::Declarative3DScene::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Declarative3DScene *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->selectionQueryPositionChanged((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Declarative3DScene::*)(const QPointF );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Declarative3DScene::selectionQueryPositionChanged)) {
                *result = 0;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<Declarative3DScene *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QPointF*>(_v) = _t->selectionQueryPosition(); break;
        case 1: *reinterpret_cast< QPoint*>(_v) = _t->invalidSelectionPoint(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<Declarative3DScene *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setSelectionQueryPosition(*reinterpret_cast< QPointF*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject QtDataVisualization::Declarative3DScene::staticMetaObject = { {
    QMetaObject::SuperData::link<Q3DScene::staticMetaObject>(),
    qt_meta_stringdata_QtDataVisualization__Declarative3DScene.data,
    qt_meta_data_QtDataVisualization__Declarative3DScene,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QtDataVisualization::Declarative3DScene::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtDataVisualization::Declarative3DScene::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QtDataVisualization__Declarative3DScene.stringdata0))
        return static_cast<void*>(this);
    return Q3DScene::qt_metacast(_clname);
}

int QtDataVisualization::Declarative3DScene::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3DScene::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void QtDataVisualization::Declarative3DScene::selectionQueryPositionChanged(const QPointF _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
