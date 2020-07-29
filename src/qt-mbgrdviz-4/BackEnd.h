#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QQmlApplicationEngine>
#include "QVtkItem.h"

class BackEnd : public QObject
{
    Q_OBJECT
public:
    explicit BackEnd(QQmlApplicationEngine *engine, QObject *parent = nullptr);

    /// Create and register singleton
    static bool registerSingleton(int argc, char **argv, QQmlApplicationEngine *engine);

    /// Specify grid file to display
    Q_INVOKABLE bool setGridFile(QUrl file);


signals:

protected:

    /// Singleton
    static BackEnd *singleInstance_;

    /// QVtkItem instantiated by QML
    QVtkItem *qVtkItem_;
};

#endif // BACKEND_H
