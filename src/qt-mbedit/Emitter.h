#ifndef MY_EMITTER_H
#define MY_EMITTER_H
#include <QObject>
#include <QString>
#include <QVariant>

/// Emitter just emits a signal
class Emitter : public QObject {

  Q_OBJECT

public:
  Emitter(void);
  
signals:

  /// Emit message to be displayed by some other component
  void showMessage(QVariant msg);

};


#endif 
