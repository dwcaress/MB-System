/****************************************************************************
** Meta object code from reading C++ file 'declarativeseries_p.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "datavisualizationqml2/declarativeseries_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'declarativeseries_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QtDataVisualization__DeclarativeBar3DSeries_t {
    QByteArrayData data[20];
    char stringdata0[428];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtDataVisualization__DeclarativeBar3DSeries_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtDataVisualization__DeclarativeBar3DSeries_t qt_meta_stringdata_QtDataVisualization__DeclarativeBar3DSeries = {
    {
QT_MOC_LITERAL(0, 0, 43), // "QtDataVisualization::Declarat..."
QT_MOC_LITERAL(1, 44, 15), // "DefaultProperty"
QT_MOC_LITERAL(2, 60, 14), // "seriesChildren"
QT_MOC_LITERAL(3, 75, 18), // "selectedBarChanged"
QT_MOC_LITERAL(4, 94, 0), // ""
QT_MOC_LITERAL(5, 95, 8), // "position"
QT_MOC_LITERAL(6, 104, 19), // "baseGradientChanged"
QT_MOC_LITERAL(7, 124, 14), // "ColorGradient*"
QT_MOC_LITERAL(8, 139, 8), // "gradient"
QT_MOC_LITERAL(9, 148, 30), // "singleHighlightGradientChanged"
QT_MOC_LITERAL(10, 179, 29), // "multiHighlightGradientChanged"
QT_MOC_LITERAL(11, 209, 24), // "handleBaseGradientUpdate"
QT_MOC_LITERAL(12, 234, 35), // "handleSingleHighlightGradient..."
QT_MOC_LITERAL(13, 270, 34), // "handleMultiHighlightGradientU..."
QT_MOC_LITERAL(14, 305, 25), // "QQmlListProperty<QObject>"
QT_MOC_LITERAL(15, 331, 11), // "selectedBar"
QT_MOC_LITERAL(16, 343, 24), // "invalidSelectionPosition"
QT_MOC_LITERAL(17, 368, 12), // "baseGradient"
QT_MOC_LITERAL(18, 381, 23), // "singleHighlightGradient"
QT_MOC_LITERAL(19, 405, 22) // "multiHighlightGradient"

    },
    "QtDataVisualization::DeclarativeBar3DSeries\0"
    "DefaultProperty\0seriesChildren\0"
    "selectedBarChanged\0\0position\0"
    "baseGradientChanged\0ColorGradient*\0"
    "gradient\0singleHighlightGradientChanged\0"
    "multiHighlightGradientChanged\0"
    "handleBaseGradientUpdate\0"
    "handleSingleHighlightGradientUpdate\0"
    "handleMultiHighlightGradientUpdate\0"
    "QQmlListProperty<QObject>\0selectedBar\0"
    "invalidSelectionPosition\0baseGradient\0"
    "singleHighlightGradient\0multiHighlightGradient"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtDataVisualization__DeclarativeBar3DSeries[] = {

 // content:
       8,       // revision
       0,       // classname
       1,   14, // classinfo
       7,   16, // methods
       6,   66, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // classinfo: key, value
       1,    2,

 // signals: name, argc, parameters, tag, flags
       3,    1,   51,    4, 0x06 /* Public */,
       6,    1,   54,    4, 0x06 /* Public */,
       9,    1,   57,    4, 0x06 /* Public */,
      10,    1,   60,    4, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    0,   63,    4, 0x0a /* Public */,
      12,    0,   64,    4, 0x0a /* Public */,
      13,    0,   65,    4, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QPointF,    5,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7,    8,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
       2, 0x80000000 | 14, 0x00095009,
      15, QMetaType::QPointF, 0x00495103,
      16, QMetaType::QPointF, 0x00095401,
      17, 0x80000000 | 7, 0x0049510b,
      18, 0x80000000 | 7, 0x0049510b,
      19, 0x80000000 | 7, 0x0049510b,

 // properties: notify_signal_id
       0,
       0,
       0,
       1,
       2,
       3,

       0        // eod
};

void QtDataVisualization::DeclarativeBar3DSeries::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DeclarativeBar3DSeries *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->selectedBarChanged((*reinterpret_cast< QPointF(*)>(_a[1]))); break;
        case 1: _t->baseGradientChanged((*reinterpret_cast< ColorGradient*(*)>(_a[1]))); break;
        case 2: _t->singleHighlightGradientChanged((*reinterpret_cast< ColorGradient*(*)>(_a[1]))); break;
        case 3: _t->multiHighlightGradientChanged((*reinterpret_cast< ColorGradient*(*)>(_a[1]))); break;
        case 4: _t->handleBaseGradientUpdate(); break;
        case 5: _t->handleSingleHighlightGradientUpdate(); break;
        case 6: _t->handleMultiHighlightGradientUpdate(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ColorGradient* >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ColorGradient* >(); break;
            }
            break;
        case 3:
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
            using _t = void (DeclarativeBar3DSeries::*)(QPointF );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeclarativeBar3DSeries::selectedBarChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DeclarativeBar3DSeries::*)(ColorGradient * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeclarativeBar3DSeries::baseGradientChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DeclarativeBar3DSeries::*)(ColorGradient * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeclarativeBar3DSeries::singleHighlightGradientChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DeclarativeBar3DSeries::*)(ColorGradient * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeclarativeBar3DSeries::multiHighlightGradientChanged)) {
                *result = 3;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 5:
        case 4:
        case 3:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ColorGradient* >(); break;
        }
    }

#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<DeclarativeBar3DSeries *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QQmlListProperty<QObject>*>(_v) = _t->seriesChildren(); break;
        case 1: *reinterpret_cast< QPointF*>(_v) = _t->selectedBar(); break;
        case 2: *reinterpret_cast< QPointF*>(_v) = _t->invalidSelectionPosition(); break;
        case 3: *reinterpret_cast< ColorGradient**>(_v) = _t->baseGradient(); break;
        case 4: *reinterpret_cast< ColorGradient**>(_v) = _t->singleHighlightGradient(); break;
        case 5: *reinterpret_cast< ColorGradient**>(_v) = _t->multiHighlightGradient(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<DeclarativeBar3DSeries *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setSelectedBar(*reinterpret_cast< QPointF*>(_v)); break;
        case 3: _t->setBaseGradient(*reinterpret_cast< ColorGradient**>(_v)); break;
        case 4: _t->setSingleHighlightGradient(*reinterpret_cast< ColorGradient**>(_v)); break;
        case 5: _t->setMultiHighlightGradient(*reinterpret_cast< ColorGradient**>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject QtDataVisualization::DeclarativeBar3DSeries::staticMetaObject = { {
    QMetaObject::SuperData::link<QBar3DSeries::staticMetaObject>(),
    qt_meta_stringdata_QtDataVisualization__DeclarativeBar3DSeries.data,
    qt_meta_data_QtDataVisualization__DeclarativeBar3DSeries,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QtDataVisualization::DeclarativeBar3DSeries::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtDataVisualization::DeclarativeBar3DSeries::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QtDataVisualization__DeclarativeBar3DSeries.stringdata0))
        return static_cast<void*>(this);
    return QBar3DSeries::qt_metacast(_clname);
}

int QtDataVisualization::DeclarativeBar3DSeries::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QBar3DSeries::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 6;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void QtDataVisualization::DeclarativeBar3DSeries::selectedBarChanged(QPointF _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtDataVisualization::DeclarativeBar3DSeries::baseGradientChanged(ColorGradient * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtDataVisualization::DeclarativeBar3DSeries::singleHighlightGradientChanged(ColorGradient * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QtDataVisualization::DeclarativeBar3DSeries::multiHighlightGradientChanged(ColorGradient * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
struct qt_meta_stringdata_QtDataVisualization__DeclarativeScatter3DSeries_t {
    QByteArrayData data[17];
    char stringdata0[389];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtDataVisualization__DeclarativeScatter3DSeries_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtDataVisualization__DeclarativeScatter3DSeries_t qt_meta_stringdata_QtDataVisualization__DeclarativeScatter3DSeries = {
    {
QT_MOC_LITERAL(0, 0, 47), // "QtDataVisualization::Declarat..."
QT_MOC_LITERAL(1, 48, 15), // "DefaultProperty"
QT_MOC_LITERAL(2, 64, 14), // "seriesChildren"
QT_MOC_LITERAL(3, 79, 19), // "baseGradientChanged"
QT_MOC_LITERAL(4, 99, 0), // ""
QT_MOC_LITERAL(5, 100, 14), // "ColorGradient*"
QT_MOC_LITERAL(6, 115, 8), // "gradient"
QT_MOC_LITERAL(7, 124, 30), // "singleHighlightGradientChanged"
QT_MOC_LITERAL(8, 155, 29), // "multiHighlightGradientChanged"
QT_MOC_LITERAL(9, 185, 24), // "handleBaseGradientUpdate"
QT_MOC_LITERAL(10, 210, 35), // "handleSingleHighlightGradient..."
QT_MOC_LITERAL(11, 246, 34), // "handleMultiHighlightGradientU..."
QT_MOC_LITERAL(12, 281, 25), // "QQmlListProperty<QObject>"
QT_MOC_LITERAL(13, 307, 12), // "baseGradient"
QT_MOC_LITERAL(14, 320, 23), // "singleHighlightGradient"
QT_MOC_LITERAL(15, 344, 22), // "multiHighlightGradient"
QT_MOC_LITERAL(16, 367, 21) // "invalidSelectionIndex"

    },
    "QtDataVisualization::DeclarativeScatter3DSeries\0"
    "DefaultProperty\0seriesChildren\0"
    "baseGradientChanged\0\0ColorGradient*\0"
    "gradient\0singleHighlightGradientChanged\0"
    "multiHighlightGradientChanged\0"
    "handleBaseGradientUpdate\0"
    "handleSingleHighlightGradientUpdate\0"
    "handleMultiHighlightGradientUpdate\0"
    "QQmlListProperty<QObject>\0baseGradient\0"
    "singleHighlightGradient\0multiHighlightGradient\0"
    "invalidSelectionIndex"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtDataVisualization__DeclarativeScatter3DSeries[] = {

 // content:
       8,       // revision
       0,       // classname
       1,   14, // classinfo
       6,   16, // methods
       5,   58, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // classinfo: key, value
       1,    2,

 // signals: name, argc, parameters, tag, flags
       3,    1,   46,    4, 0x06 /* Public */,
       7,    1,   49,    4, 0x06 /* Public */,
       8,    1,   52,    4, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,   55,    4, 0x0a /* Public */,
      10,    0,   56,    4, 0x0a /* Public */,
      11,    0,   57,    4, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
       2, 0x80000000 | 12, 0x00095009,
      13, 0x80000000 | 5, 0x0049510b,
      14, 0x80000000 | 5, 0x0049510b,
      15, 0x80000000 | 5, 0x0049510b,
      16, QMetaType::Int, 0x00095401,

 // properties: notify_signal_id
       0,
       0,
       1,
       2,
       0,

       0        // eod
};

void QtDataVisualization::DeclarativeScatter3DSeries::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DeclarativeScatter3DSeries *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->baseGradientChanged((*reinterpret_cast< ColorGradient*(*)>(_a[1]))); break;
        case 1: _t->singleHighlightGradientChanged((*reinterpret_cast< ColorGradient*(*)>(_a[1]))); break;
        case 2: _t->multiHighlightGradientChanged((*reinterpret_cast< ColorGradient*(*)>(_a[1]))); break;
        case 3: _t->handleBaseGradientUpdate(); break;
        case 4: _t->handleSingleHighlightGradientUpdate(); break;
        case 5: _t->handleMultiHighlightGradientUpdate(); break;
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
        case 2:
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
            using _t = void (DeclarativeScatter3DSeries::*)(ColorGradient * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeclarativeScatter3DSeries::baseGradientChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DeclarativeScatter3DSeries::*)(ColorGradient * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeclarativeScatter3DSeries::singleHighlightGradientChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DeclarativeScatter3DSeries::*)(ColorGradient * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeclarativeScatter3DSeries::multiHighlightGradientChanged)) {
                *result = 2;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
        case 2:
        case 1:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ColorGradient* >(); break;
        }
    }

#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<DeclarativeScatter3DSeries *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QQmlListProperty<QObject>*>(_v) = _t->seriesChildren(); break;
        case 1: *reinterpret_cast< ColorGradient**>(_v) = _t->baseGradient(); break;
        case 2: *reinterpret_cast< ColorGradient**>(_v) = _t->singleHighlightGradient(); break;
        case 3: *reinterpret_cast< ColorGradient**>(_v) = _t->multiHighlightGradient(); break;
        case 4: *reinterpret_cast< int*>(_v) = _t->invalidSelectionIndex(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<DeclarativeScatter3DSeries *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setBaseGradient(*reinterpret_cast< ColorGradient**>(_v)); break;
        case 2: _t->setSingleHighlightGradient(*reinterpret_cast< ColorGradient**>(_v)); break;
        case 3: _t->setMultiHighlightGradient(*reinterpret_cast< ColorGradient**>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject QtDataVisualization::DeclarativeScatter3DSeries::staticMetaObject = { {
    QMetaObject::SuperData::link<QScatter3DSeries::staticMetaObject>(),
    qt_meta_stringdata_QtDataVisualization__DeclarativeScatter3DSeries.data,
    qt_meta_data_QtDataVisualization__DeclarativeScatter3DSeries,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QtDataVisualization::DeclarativeScatter3DSeries::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtDataVisualization::DeclarativeScatter3DSeries::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QtDataVisualization__DeclarativeScatter3DSeries.stringdata0))
        return static_cast<void*>(this);
    return QScatter3DSeries::qt_metacast(_clname);
}

int QtDataVisualization::DeclarativeScatter3DSeries::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QScatter3DSeries::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
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
void QtDataVisualization::DeclarativeScatter3DSeries::baseGradientChanged(ColorGradient * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtDataVisualization::DeclarativeScatter3DSeries::singleHighlightGradientChanged(ColorGradient * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtDataVisualization::DeclarativeScatter3DSeries::multiHighlightGradientChanged(ColorGradient * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
struct qt_meta_stringdata_QtDataVisualization__DeclarativeSurface3DSeries_t {
    QByteArrayData data[20];
    char stringdata0[436];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QtDataVisualization__DeclarativeSurface3DSeries_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QtDataVisualization__DeclarativeSurface3DSeries_t qt_meta_stringdata_QtDataVisualization__DeclarativeSurface3DSeries = {
    {
QT_MOC_LITERAL(0, 0, 47), // "QtDataVisualization::Declarat..."
QT_MOC_LITERAL(1, 48, 15), // "DefaultProperty"
QT_MOC_LITERAL(2, 64, 14), // "seriesChildren"
QT_MOC_LITERAL(3, 79, 20), // "selectedPointChanged"
QT_MOC_LITERAL(4, 100, 0), // ""
QT_MOC_LITERAL(5, 101, 8), // "position"
QT_MOC_LITERAL(6, 110, 19), // "baseGradientChanged"
QT_MOC_LITERAL(7, 130, 14), // "ColorGradient*"
QT_MOC_LITERAL(8, 145, 8), // "gradient"
QT_MOC_LITERAL(9, 154, 30), // "singleHighlightGradientChanged"
QT_MOC_LITERAL(10, 185, 29), // "multiHighlightGradientChanged"
QT_MOC_LITERAL(11, 215, 24), // "handleBaseGradientUpdate"
QT_MOC_LITERAL(12, 240, 35), // "handleSingleHighlightGradient..."
QT_MOC_LITERAL(13, 276, 34), // "handleMultiHighlightGradientU..."
QT_MOC_LITERAL(14, 311, 25), // "QQmlListProperty<QObject>"
QT_MOC_LITERAL(15, 337, 13), // "selectedPoint"
QT_MOC_LITERAL(16, 351, 24), // "invalidSelectionPosition"
QT_MOC_LITERAL(17, 376, 12), // "baseGradient"
QT_MOC_LITERAL(18, 389, 23), // "singleHighlightGradient"
QT_MOC_LITERAL(19, 413, 22) // "multiHighlightGradient"

    },
    "QtDataVisualization::DeclarativeSurface3DSeries\0"
    "DefaultProperty\0seriesChildren\0"
    "selectedPointChanged\0\0position\0"
    "baseGradientChanged\0ColorGradient*\0"
    "gradient\0singleHighlightGradientChanged\0"
    "multiHighlightGradientChanged\0"
    "handleBaseGradientUpdate\0"
    "handleSingleHighlightGradientUpdate\0"
    "handleMultiHighlightGradientUpdate\0"
    "QQmlListProperty<QObject>\0selectedPoint\0"
    "invalidSelectionPosition\0baseGradient\0"
    "singleHighlightGradient\0multiHighlightGradient"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QtDataVisualization__DeclarativeSurface3DSeries[] = {

 // content:
       8,       // revision
       0,       // classname
       1,   14, // classinfo
       7,   16, // methods
       6,   66, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // classinfo: key, value
       1,    2,

 // signals: name, argc, parameters, tag, flags
       3,    1,   51,    4, 0x06 /* Public */,
       6,    1,   54,    4, 0x06 /* Public */,
       9,    1,   57,    4, 0x06 /* Public */,
      10,    1,   60,    4, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    0,   63,    4, 0x0a /* Public */,
      12,    0,   64,    4, 0x0a /* Public */,
      13,    0,   65,    4, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QPointF,    5,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7,    8,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
       2, 0x80000000 | 14, 0x00095009,
      15, QMetaType::QPointF, 0x00495103,
      16, QMetaType::QPointF, 0x00095401,
      17, 0x80000000 | 7, 0x0049510b,
      18, 0x80000000 | 7, 0x0049510b,
      19, 0x80000000 | 7, 0x0049510b,

 // properties: notify_signal_id
       0,
       0,
       0,
       1,
       2,
       3,

       0        // eod
};

void QtDataVisualization::DeclarativeSurface3DSeries::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DeclarativeSurface3DSeries *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->selectedPointChanged((*reinterpret_cast< QPointF(*)>(_a[1]))); break;
        case 1: _t->baseGradientChanged((*reinterpret_cast< ColorGradient*(*)>(_a[1]))); break;
        case 2: _t->singleHighlightGradientChanged((*reinterpret_cast< ColorGradient*(*)>(_a[1]))); break;
        case 3: _t->multiHighlightGradientChanged((*reinterpret_cast< ColorGradient*(*)>(_a[1]))); break;
        case 4: _t->handleBaseGradientUpdate(); break;
        case 5: _t->handleSingleHighlightGradientUpdate(); break;
        case 6: _t->handleMultiHighlightGradientUpdate(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ColorGradient* >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ColorGradient* >(); break;
            }
            break;
        case 3:
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
            using _t = void (DeclarativeSurface3DSeries::*)(QPointF );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeclarativeSurface3DSeries::selectedPointChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DeclarativeSurface3DSeries::*)(ColorGradient * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeclarativeSurface3DSeries::baseGradientChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DeclarativeSurface3DSeries::*)(ColorGradient * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeclarativeSurface3DSeries::singleHighlightGradientChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DeclarativeSurface3DSeries::*)(ColorGradient * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DeclarativeSurface3DSeries::multiHighlightGradientChanged)) {
                *result = 3;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 5:
        case 4:
        case 3:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< ColorGradient* >(); break;
        }
    }

#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<DeclarativeSurface3DSeries *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QQmlListProperty<QObject>*>(_v) = _t->seriesChildren(); break;
        case 1: *reinterpret_cast< QPointF*>(_v) = _t->selectedPoint(); break;
        case 2: *reinterpret_cast< QPointF*>(_v) = _t->invalidSelectionPosition(); break;
        case 3: *reinterpret_cast< ColorGradient**>(_v) = _t->baseGradient(); break;
        case 4: *reinterpret_cast< ColorGradient**>(_v) = _t->singleHighlightGradient(); break;
        case 5: *reinterpret_cast< ColorGradient**>(_v) = _t->multiHighlightGradient(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<DeclarativeSurface3DSeries *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setSelectedPoint(*reinterpret_cast< QPointF*>(_v)); break;
        case 3: _t->setBaseGradient(*reinterpret_cast< ColorGradient**>(_v)); break;
        case 4: _t->setSingleHighlightGradient(*reinterpret_cast< ColorGradient**>(_v)); break;
        case 5: _t->setMultiHighlightGradient(*reinterpret_cast< ColorGradient**>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject QtDataVisualization::DeclarativeSurface3DSeries::staticMetaObject = { {
    QMetaObject::SuperData::link<QSurface3DSeries::staticMetaObject>(),
    qt_meta_stringdata_QtDataVisualization__DeclarativeSurface3DSeries.data,
    qt_meta_data_QtDataVisualization__DeclarativeSurface3DSeries,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QtDataVisualization::DeclarativeSurface3DSeries::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtDataVisualization::DeclarativeSurface3DSeries::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QtDataVisualization__DeclarativeSurface3DSeries.stringdata0))
        return static_cast<void*>(this);
    return QSurface3DSeries::qt_metacast(_clname);
}

int QtDataVisualization::DeclarativeSurface3DSeries::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QSurface3DSeries::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 6;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void QtDataVisualization::DeclarativeSurface3DSeries::selectedPointChanged(QPointF _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QtDataVisualization::DeclarativeSurface3DSeries::baseGradientChanged(ColorGradient * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QtDataVisualization::DeclarativeSurface3DSeries::singleHighlightGradientChanged(ColorGradient * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QtDataVisualization::DeclarativeSurface3DSeries::multiHighlightGradientChanged(ColorGradient * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
