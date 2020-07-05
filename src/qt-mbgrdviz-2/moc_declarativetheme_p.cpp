/****************************************************************************
** Meta object code from reading C++ file 'declarativetheme_p.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "datavisualizationqml2/declarativetheme_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'declarativetheme_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtDataVisualization__DeclarativeTheme3D_t {
    QByteArrayData data[15];
    char stringdata0[321];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtDataVisualization__DeclarativeTheme3D_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtDataVisualization__DeclarativeTheme3D_t qt_meta_stringdata_QtDataVisualization__DeclarativeTheme3D = {
    {
QT_MOC_LITERAL(0, 0, 39), // "QtDataVisualization::Declarat..."
QT_MOC_LITERAL(1, 40, 15), // "DefaultProperty"
QT_MOC_LITERAL(2, 56, 13), // "themeChildren"
QT_MOC_LITERAL(3, 70, 30), // "singleHighlightGradientChanged"
QT_MOC_LITERAL(4, 101, 0), // ""
QT_MOC_LITERAL(5, 102, 14), // "ColorGradient*"
QT_MOC_LITERAL(6, 117, 8), // "gradient"
QT_MOC_LITERAL(7, 126, 29), // "multiHighlightGradientChanged"
QT_MOC_LITERAL(8, 156, 25), // "QQmlListProperty<QObject>"
QT_MOC_LITERAL(9, 182, 10), // "baseColors"
QT_MOC_LITERAL(10, 193, 34), // "QQmlListProperty<DeclarativeC..."
QT_MOC_LITERAL(11, 228, 13), // "baseGradients"
QT_MOC_LITERAL(12, 242, 31), // "QQmlListProperty<ColorGradient>"
QT_MOC_LITERAL(13, 274, 23), // "singleHighlightGradient"
QT_MOC_LITERAL(14, 298, 22) // "multiHighlightGradient"

    },
    "QtDataVisualization::DeclarativeTheme3D\0"
    "DefaultProperty\0themeChildren\0"
    "singleHighlightGradientChanged\0\0"
    "ColorGradient*\0gradient\0"
    "multiHighlightGradientChanged\0"
    "QQmlListProperty<QObject>\0baseColors\0"
    "QQmlListProperty<DeclarativeColor>\0"
    "baseGradients\0QQmlListProperty<ColorGradient>\0"
    "singleHighlightGradient\0multiHighlightGradient"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtDataVisualization__DeclarativeTheme3D[] = {

 // content:
       8,       // revision
       0,       // classname
       1,   14, // classinfo
       2,   16, // methods
       5,   32, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // classinfo: key, value
       1,    2,

 // signals: name, argc, parameters, tag, flags
       3,    1,   26,    4, 0x06 /* Public */,
       7,    1,   29,    4, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,

 // properties: name, type, flags
       2, 0x80000000 | 8, 0x00095009,
       9, 0x80000000 | 10, 0x00095009,
      11, 0x80000000 | 12, 0x00095009,
      13, 0x80000000 | 5, 0x0049510b,
      14, 0x80000000 | 5, 0x0049510b,

 // properties: notify_signal_id
       0,
       0,
       0,
       0,
       1,

       0        // eod
};

void QtDataVisualization::DeclarativeTheme3D::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DeclarativeTheme3D *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->singleHighlightGradientChanged((*reinterpret_cast< ColorGradient*(*)>(_a[1]))); break;
        case 1: _t->multiHighlightGradientChanged((*reinterpret_cast< ColorGradient*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ColorGradient* >(); break;
            }
            break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ColorGradient* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DeclarativeTheme3D::*)(ColorGradient * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeclarativeTheme3D::singleHighlightGradientChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DeclarativeTheme3D::*)(ColorGradient * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeclarativeTheme3D::multiHighlightGradientChanged)) {
                *result = 1;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 4:
        case 3:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ColorGradient* >(); break;
        }
    }

#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<DeclarativeTheme3D *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QQmlListProperty<QObject>*>(_v) = _t->themeChildren(); break;
        case 1: *reinterpret_cast< QQmlListProperty<DeclarativeColor>*>(_v) = _t->baseColors(); break;
        case 2: *reinterpret_cast< QQmlListProperty<ColorGradient>*>(_v) = _t->baseGradients(); break;
        case 3: *reinterpret_cast< ColorGradient**>(_v) = _t->singleHighlightGradient(); break;
        case 4: *reinterpret_cast< ColorGradient**>(_v) = _t->multiHighlightGradient(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<DeclarativeTheme3D *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 3: _t->setSingleHighlightGradient(*reinterpret_cast< ColorGradient**>(_v)); break;
        case 4: _t->setMultiHighlightGradient(*reinterpret_cast< ColorGradient**>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject QtDataVisualization::DeclarativeTheme3D::staticMetaObject = { {
    QMetaObject::SuperData::link<Q3DTheme::staticMetaObject>(),
    qt_meta_stringdata_QtDataVisualization__DeclarativeTheme3D.data,
    qt_meta_data_QtDataVisualization__DeclarativeTheme3D,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QtDataVisualization::DeclarativeTheme3D::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtDataVisualization::DeclarativeTheme3D::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QtDataVisualization__DeclarativeTheme3D.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QQmlParserStatus"))
        return static_cast< QQmlParserStatus*>(this);
    if (!strcmp(_clname, "org.qt-project.Qt.QQmlParserStatus"))
        return static_cast< QQmlParserStatus*>(this);
    return Q3DTheme::qt_metacast(_clname);
}

int QtDataVisualization::DeclarativeTheme3D::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3DTheme::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 5;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 5;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void QtDataVisualization::DeclarativeTheme3D::singleHighlightGradientChanged(ColorGradient * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtDataVisualization::DeclarativeTheme3D::multiHighlightGradientChanged(ColorGradient * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
