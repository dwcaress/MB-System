/****************************************************************************
** Meta object code from reading C++ file 'Camera.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "Camera.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Camera.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Camera_t {
    QByteArrayData data[24];
    char stringdata0[273];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Camera_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Camera_t qt_meta_stringdata_Camera = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Camera"
QT_MOC_LITERAL(1, 7, 14), // "azimuthChanged"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 7), // "azimuth"
QT_MOC_LITERAL(4, 31, 15), // "distanceChanged"
QT_MOC_LITERAL(5, 47, 8), // "distance"
QT_MOC_LITERAL(6, 56, 16), // "elevationChanged"
QT_MOC_LITERAL(7, 73, 9), // "elevation"
QT_MOC_LITERAL(8, 83, 14), // "xOffsetChanged"
QT_MOC_LITERAL(9, 98, 6), // "offset"
QT_MOC_LITERAL(10, 105, 14), // "yOffsetChanged"
QT_MOC_LITERAL(11, 120, 18), // "forceRenderChanged"
QT_MOC_LITERAL(12, 139, 5), // "value"
QT_MOC_LITERAL(13, 145, 10), // "setAzimuth"
QT_MOC_LITERAL(14, 156, 11), // "setDistance"
QT_MOC_LITERAL(15, 168, 12), // "setElevation"
QT_MOC_LITERAL(16, 181, 14), // "setMaxDistance"
QT_MOC_LITERAL(17, 196, 11), // "maxDistance"
QT_MOC_LITERAL(18, 208, 10), // "setXOffset"
QT_MOC_LITERAL(19, 219, 10), // "setYOffset"
QT_MOC_LITERAL(20, 230, 14), // "setForceRender"
QT_MOC_LITERAL(21, 245, 7), // "xOffset"
QT_MOC_LITERAL(22, 253, 7), // "yOffset"
QT_MOC_LITERAL(23, 261, 11) // "forceRender"

    },
    "Camera\0azimuthChanged\0\0azimuth\0"
    "distanceChanged\0distance\0elevationChanged\0"
    "elevation\0xOffsetChanged\0offset\0"
    "yOffsetChanged\0forceRenderChanged\0"
    "value\0setAzimuth\0setDistance\0setElevation\0"
    "setMaxDistance\0maxDistance\0setXOffset\0"
    "setYOffset\0setForceRender\0xOffset\0"
    "yOffset\0forceRender"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Camera[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       7,  118, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   79,    2, 0x06 /* Public */,
       4,    1,   82,    2, 0x06 /* Public */,
       6,    1,   85,    2, 0x06 /* Public */,
       8,    1,   88,    2, 0x06 /* Public */,
      10,    1,   91,    2, 0x06 /* Public */,
      11,    1,   94,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    1,   97,    2, 0x0a /* Public */,
      14,    1,  100,    2, 0x0a /* Public */,
      15,    1,  103,    2, 0x0a /* Public */,
      16,    1,  106,    2, 0x0a /* Public */,
      18,    1,  109,    2, 0x0a /* Public */,
      19,    1,  112,    2, 0x0a /* Public */,
      20,    1,  115,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Float,    3,
    QMetaType::Void, QMetaType::Float,    5,
    QMetaType::Void, QMetaType::Float,    7,
    QMetaType::Void, QMetaType::Float,    9,
    QMetaType::Void, QMetaType::Float,    9,
    QMetaType::Void, QMetaType::Bool,   12,

 // slots: parameters
    QMetaType::Void, QMetaType::Float,    3,
    QMetaType::Void, QMetaType::Float,    5,
    QMetaType::Void, QMetaType::Float,    7,
    QMetaType::Void, QMetaType::Float,   17,
    QMetaType::Void, QMetaType::Float,    9,
    QMetaType::Void, QMetaType::Float,    9,
    QMetaType::Void, QMetaType::Bool,   12,

 // properties: name, type, flags
       3, QMetaType::Float, 0x00495103,
       7, QMetaType::Float, 0x00495103,
       5, QMetaType::Float, 0x00495103,
      21, QMetaType::Float, 0x00495103,
      22, QMetaType::Float, 0x00495103,
      23, QMetaType::Bool, 0x00495103,
      17, QMetaType::Float, 0x00095001,

 // properties: notify_signal_id
       0,
       2,
       1,
       3,
       4,
       5,
       0,

       0        // eod
};

void Camera::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Camera *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->azimuthChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 1: _t->distanceChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 2: _t->elevationChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 3: _t->xOffsetChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 4: _t->yOffsetChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 5: _t->forceRenderChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->setAzimuth((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 7: _t->setDistance((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 8: _t->setElevation((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 9: _t->setMaxDistance((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 10: _t->setXOffset((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 11: _t->setYOffset((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 12: _t->setForceRender((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Camera::*)(float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Camera::azimuthChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Camera::*)(float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Camera::distanceChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Camera::*)(float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Camera::elevationChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Camera::*)(float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Camera::xOffsetChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (Camera::*)(float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Camera::yOffsetChanged)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (Camera::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Camera::forceRenderChanged)) {
                *result = 5;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<Camera *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< float*>(_v) = _t->azimuth(); break;
        case 1: *reinterpret_cast< float*>(_v) = _t->elevation(); break;
        case 2: *reinterpret_cast< float*>(_v) = _t->distance(); break;
        case 3: *reinterpret_cast< float*>(_v) = _t->xOffset(); break;
        case 4: *reinterpret_cast< float*>(_v) = _t->yOffset(); break;
        case 5: *reinterpret_cast< bool*>(_v) = _t->forceRender(); break;
        case 6: *reinterpret_cast< float*>(_v) = _t->maxDistance(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<Camera *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setAzimuth(*reinterpret_cast< float*>(_v)); break;
        case 1: _t->setElevation(*reinterpret_cast< float*>(_v)); break;
        case 2: _t->setDistance(*reinterpret_cast< float*>(_v)); break;
        case 3: _t->setXOffset(*reinterpret_cast< float*>(_v)); break;
        case 4: _t->setYOffset(*reinterpret_cast< float*>(_v)); break;
        case 5: _t->setForceRender(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject Camera::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_Camera.data,
    qt_meta_data_Camera,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Camera::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Camera::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Camera.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Camera::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 7;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void Camera::azimuthChanged(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Camera::distanceChanged(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Camera::elevationChanged(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Camera::xOffsetChanged(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Camera::yOffsetChanged(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Camera::forceRenderChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
