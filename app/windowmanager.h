#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>

// Structure to pass our QList pointer through the EnumWindows callback.
struct GrabData {
  QList<QPixmap>* pixmaps;
  QList<std::string>* names;
};

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

  std::pair<QList<QPixmap>, QList<std::string>> grab_pixmaps_of_active_windows();

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
