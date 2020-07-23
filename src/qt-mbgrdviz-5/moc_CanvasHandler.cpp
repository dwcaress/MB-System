/****************************************************************************
** Meta object code from reading C++ file 'CanvasHandler.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "CanvasHandler.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CanvasHandler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CanvasHandler_t {
    QByteArrayData data[31];
    char stringdata0[464];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CanvasHandler_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CanvasHandler_t qt_meta_stringdata_CanvasHandler = {
    {
QT_MOC_LITERAL(0, 0, 13), // "CanvasHandler"
QT_MOC_LITERAL(1, 14, 21), // "showFileDialogChanged"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 22), // "isModelSelectedChanged"
QT_MOC_LITERAL(4, 60, 29), // "selectedModelPositionXChanged"
QT_MOC_LITERAL(5, 90, 29), // "selectedModelPositionYChanged"
QT_MOC_LITERAL(6, 120, 16), // "startApplication"
QT_MOC_LITERAL(7, 137, 9), // "openModel"
QT_MOC_LITERAL(8, 147, 4), // "path"
QT_MOC_LITERAL(9, 152, 15), // "mousePressEvent"
QT_MOC_LITERAL(10, 168, 6), // "button"
QT_MOC_LITERAL(11, 175, 6), // "mouseX"
QT_MOC_LITERAL(12, 182, 6), // "mouseY"
QT_MOC_LITERAL(13, 189, 14), // "mouseMoveEvent"
QT_MOC_LITERAL(14, 204, 17), // "mouseReleaseEvent"
QT_MOC_LITERAL(15, 222, 23), // "setModelsRepresentation"
QT_MOC_LITERAL(16, 246, 20), // "representationOption"
QT_MOC_LITERAL(17, 267, 16), // "setModelsOpacity"
QT_MOC_LITERAL(18, 284, 7), // "opacity"
QT_MOC_LITERAL(19, 292, 23), // "setGouraudInterpolation"
QT_MOC_LITERAL(20, 316, 20), // "gouraudInterpolation"
QT_MOC_LITERAL(21, 337, 14), // "setModelColorR"
QT_MOC_LITERAL(22, 352, 6), // "colorR"
QT_MOC_LITERAL(23, 359, 14), // "setModelColorG"
QT_MOC_LITERAL(24, 374, 6), // "colorG"
QT_MOC_LITERAL(25, 381, 14), // "setModelColorB"
QT_MOC_LITERAL(26, 396, 6), // "colorB"
QT_MOC_LITERAL(27, 403, 14), // "showFileDialog"
QT_MOC_LITERAL(28, 418, 15), // "isModelSelected"
QT_MOC_LITERAL(29, 434, 14), // "modelPositionX"
QT_MOC_LITERAL(30, 449, 14) // "modelPositionY"

    },
    "CanvasHandler\0showFileDialogChanged\0"
    "\0isModelSelectedChanged\0"
    "selectedModelPositionXChanged\0"
    "selectedModelPositionYChanged\0"
    "startApplication\0openModel\0path\0"
    "mousePressEvent\0button\0mouseX\0mouseY\0"
    "mouseMoveEvent\0mouseReleaseEvent\0"
    "setModelsRepresentation\0representationOption\0"
    "setModelsOpacity\0opacity\0"
    "setGouraudInterpolation\0gouraudInterpolation\0"
    "setModelColorR\0colorR\0setModelColorG\0"
    "colorG\0setModelColorB\0colorB\0"
    "showFileDialog\0isModelSelected\0"
    "modelPositionX\0modelPositionY"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CanvasHandler[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       4,  136, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   89,    2, 0x06 /* Public */,
       3,    0,   90,    2, 0x06 /* Public */,
       4,    0,   91,    2, 0x06 /* Public */,
       5,    0,   92,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   93,    2, 0x0a /* Public */,

 // methods: name, argc, parameters, tag, flags
       7,    1,   94,    2, 0x02 /* Public */,
       9,    3,   97,    2, 0x02 /* Public */,
      13,    3,  104,    2, 0x02 /* Public */,
      14,    3,  111,    2, 0x02 /* Public */,
      15,    1,  118,    2, 0x02 /* Public */,
      17,    1,  121,    2, 0x02 /* Public */,
      19,    1,  124,    2, 0x02 /* Public */,
      21,    1,  127,    2, 0x02 /* Public */,
      23,    1,  130,    2, 0x02 /* Public */,
      25,    1,  133,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, QMetaType::QUrl,    8,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,   10,   11,   12,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,   10,   11,   12,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,   10,   11,   12,
    QMetaType::Void, QMetaType::Int,   16,
    QMetaType::Void, QMetaType::Double,   18,
    QMetaType::Void, QMetaType::Bool,   20,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Int,   26,

 // properties: name, type, flags
      27, QMetaType::Bool, 0x00495003,
      28, QMetaType::Bool, 0x00495001,
      29, QMetaType::Double, 0x00495001,
      30, QMetaType::Double, 0x00495001,

 // properties: notify_signal_id
       0,
       1,
       2,
       3,

       0        // eod
};

void CanvasHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CanvasHandler *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->showFileDialogChanged(); break;
        case 1: _t->isModelSelectedChanged(); break;
        case 2: _t->selectedModelPositionXChanged(); break;
        case 3: _t->selectedModelPositionYChanged(); break;
        case 4: _t->startApplication(); break;
        case 5: _t->openModel((*reinterpret_cast< const QUrl(*)>(_a[1]))); break;
        case 6: _t->mousePressEvent((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2])),(*reinterpret_cast< const int(*)>(_a[3]))); break;
        case 7: _t->mouseMoveEvent((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2])),(*reinterpret_cast< const int(*)>(_a[3]))); break;
        case 8: _t->mouseReleaseEvent((*reinterpret_cast< const int(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2])),(*reinterpret_cast< const int(*)>(_a[3]))); break;
        case 9: _t->setModelsRepresentation((*reinterpret_cast< const int(*)>(_a[1]))); break;
        case 10: _t->setModelsOpacity((*reinterpret_cast< const double(*)>(_a[1]))); break;
        case 11: _t->setGouraudInterpolation((*reinterpret_cast< const bool(*)>(_a[1]))); break;
        case 12: _t->setModelColorR((*reinterpret_cast< const int(*)>(_a[1]))); break;
        case 13: _t->setModelColorG((*reinterpret_cast< const int(*)>(_a[1]))); break;
        case 14: _t->setModelColorB((*reinterpret_cast< const int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CanvasHandler::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CanvasHandler::showFileDialogChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CanvasHandler::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CanvasHandler::isModelSelectedChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CanvasHandler::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CanvasHandler::selectedModelPositionXChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CanvasHandler::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CanvasHandler::selectedModelPositionYChanged)) {
                *result = 3;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<CanvasHandler *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->m_showFileDialog; break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->getIsModelSelected(); break;
        case 2: *reinterpret_cast< double*>(_v) = _t->getSelectedModelPositionX(); break;
        case 3: *reinterpret_cast< double*>(_v) = _t->getSelectedModelPositionY(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<CanvasHandler *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0:
            if (_t->m_showFileDialog != *reinterpret_cast< bool*>(_v)) {
                _t->m_showFileDialog = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->showFileDialogChanged();
            }
            break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject CanvasHandler::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CanvasHandler.data,
    qt_meta_data_CanvasHandler,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CanvasHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CanvasHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CanvasHandler.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CanvasHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 15;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 4;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 4;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void CanvasHandler::showFileDialogChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CanvasHandler::isModelSelectedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CanvasHandler::selectedModelPositionXChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void CanvasHandler::selectedModelPositionYChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
