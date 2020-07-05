#ifndef MYTIMER_H
#define MYTIMER_H

#include <QObject>
#include <QTimer>
#include <QQuickWindow>

/**
MyTimer lists loaded child objects after specified number of seconds
 */
class MyTimer : public QObject {
    Q_OBJECT
public:
    MyTimer(QQuickWindow *root) {
        m_root = root;
    }

    inline void start(int seconds) {
        QTimer::singleShot(seconds * 1000, this, SLOT(timerExpired()));
    }

    QQuickWindow *m_root;

private slots:
    inline void timerExpired() {
        // List loaded child obects
        QList<QObject *>children = m_root->findChildren<QObject *>();
        qDebug() << "found " << children.size() << "children";
        for (int i = 0; i < children.size(); i++) {
            qDebug() << "child name " << children[i]->objectName() << ", class " << children[i]->metaObject()->className();
        }
    }

};

#endif // MYTIMER_H
