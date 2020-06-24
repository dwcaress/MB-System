#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QQmlEngine>
#include <QJSEngine>
#include <QUrl>
#include <QQuickWindow>
#include <Q3DSurface>
#include <QQmlApplicationEngine>
#include "datavisualizationqml2/declarativesurface_p.h"
#include "TopographicSeries.h"

/// BackEnd contains C++ "business logic" for bathymetry/topography viewer
class BackEnd : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BackEnd)

public:
    explicit BackEnd(QObject *parent = nullptr);

    /// Create/register singleton. Return true on success, else false
    static bool registerSingleton(int argc, char **argv, QQmlEngine *qmlEngine);

    Q_INVOKABLE void setGridFile(QUrl file);
  Q_INVOKABLE void test();

    static QObject *qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine) {
        Q_UNUSED(engine);
        Q_UNUSED(scriptEngine)

        QObject *obj = new BackEnd();
        fprintf(stderr, "backend singleton ptr: %p\n", obj);

        return obj;
    }

    /// Get applications options from command line
    virtual bool getOptions(int argc, char **argv);

    /// print usage to stderr
    virtual void printUsage();

    /// Singleton instance
    static BackEnd *m_instance;

signals:

public slots:
protected:

    /// Set member pointers to QML items
    bool getQmlItems();

    /// Adjust min and max by specified padFactor
    void padRange(double *min, double *max, double padFactor) {
        *min -= (*min * padFactor);
        *max += (*max * padFactor);
    }

    /// Surface (from QML)
    QtDataVisualization::DeclarativeSurface *m_surface;

    /// Topographic/bathymetric data
    TopographicSeries *m_topographicSeries;

  /// Displayed file name
  QObject *m_selectedFileText;

    /// Current grid file
    char *m_gridFile;

    static void listProperties(QObject *object) {
        const QMetaObject *metaobject = object->metaObject();
        int count = metaobject->propertyCount();
        for (int i=0; i<count; ++i) {
            QMetaProperty metaproperty = metaobject->property(i);
            const char *name = metaproperty.name();
            QVariant value = object->property(name);
            qDebug() << "property " << name << " = " << value;
        }
    }

};

/// Global objects
extern QQuickWindow *g_rootWindow;
extern QQmlApplicationEngine *g_appEngine;


#endif // BACKEND_H
