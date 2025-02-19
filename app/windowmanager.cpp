#include "WindowManager.h"
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_MAC)
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#endif

// A simple hello method for demonstration.
void WindowManager::hello()
{
  qDebug() << "Hello, World from WindowManager!";
}

#ifdef Q_OS_WIN
// Helper callback for EnumWindows on Windows.
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
  QStringList *windowsList = reinterpret_cast<QStringList *>(lParam);

  // Check if the window is visible.
  if (IsWindowVisible(hwnd))
  {
    int length = GetWindowTextLengthW(hwnd);
    if (length > 0)
    {
      // Allocate a buffer for the window title.
      wchar_t *buffer = new wchar_t[length + 1];
      GetWindowTextW(hwnd, buffer, length + 1);
      QString title = QString::fromWCharArray(buffer);
      delete[] buffer;
      if (!title.isEmpty())
      {
        windowsList->append(title);
      }
    }
  }
  return TRUE; // Continue enumeration.
}
#endif

QStringList WindowManager::get_windows()
{
  QStringList windows;

#ifdef Q_OS_WIN
  // Use the Win32 API to enumerate all top-level windows.
  EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));

#elif defined(Q_OS_MAC)
  // Use the CoreGraphics API to get information about on-screen windows.
  CFArrayRef windowInfoArray = CGWindowListCopyWindowInfo(
      kCGWindowListOptionOnScreenOnly, kCGNullWindowID);

  if (windowInfoArray)
  {
    CFIndex count = CFArrayGetCount(windowInfoArray);
    for (CFIndex i = 0; i < count; i++)
    {
      CFDictionaryRef dict =
          (CFDictionaryRef) CFArrayGetValueAtIndex(windowInfoArray, i);
      // Retrieve the window title.
      CFStringRef titleRef =
          (CFStringRef) CFDictionaryGetValue(dict, kCGWindowName);
      if (titleRef)
      {
        // Convert the CFStringRef to a QString.
        char buffer[256];
        if (CFStringGetCString(titleRef, buffer, sizeof(buffer),
                               kCFStringEncodingUTF8))
        {
          QString title = QString::fromUtf8(buffer);
          if (!title.isEmpty())
          {
            windows.append(title);
          }
        }
      }
    }
    CFRelease(windowInfoArray);
  }
#else
  // For platforms other than Windows and macOS, we return an empty list.
  qDebug() << "getWindows() is not implemented on this platform.";
#endif

  return windows;
}
