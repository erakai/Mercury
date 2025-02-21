#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>

// Inherits from QObject if you need signals/slots.
class WindowManager : public QObject {
  Q_OBJECT
public:
  // Provide a static accessor method.
  static WindowManager& instance() {
    // C++11 guarantees thread-safe initialization of local statics.
    static WindowManager instance;
    return instance;
  }

  // Other public methods...
  void hello();
  QStringList get_windows();
  void enumerateProcessesAndCaptureMainWindow();

private:
  // Private constructor prevents direct instantiation.
  explicit WindowManager(QObject* parent = nullptr) : QObject(parent) {
    // Initialization code...
  }
  // Destructor (can be defaulted or customized)
  ~WindowManager() = default;

  // Delete copy constructor and assignment operator to prevent copies.
  WindowManager(const WindowManager&) = delete;
  WindowManager& operator=(const WindowManager&) = delete;
};

#endif // WINDOWMANAGER_H
