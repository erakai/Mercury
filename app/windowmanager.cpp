#include "WindowManager.h"
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QPixmap>

#ifdef Q_OS_WIN
#include <stdio.h>
#include <windows.h>
#include <TlHelp32.h>

// This helper function captures the given window using PrintWindow,
// checks if the capture is non-black, and returns the QPixmap.
// If the capture fails or is completely black, an empty QPixmap is returned.
static QPixmap captureWindowAsPixmap(HWND hwnd)
{
  // Get window rectangle.
  RECT rect;
  if (!GetWindowRect(hwnd, &rect))
    return QPixmap();
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;
  if (width <= 0 || height <= 0)
    return QPixmap();

  // Get the window's device context.
  HDC hdcWindow = GetDC(hwnd);
  if (!hdcWindow)
    return QPixmap();

  // Create a memory DC and compatible bitmap.
  HDC hdcMem = CreateCompatibleDC(hdcWindow);
  HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, width, height);
  if (!hBitmap)
  {
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);
    return QPixmap();
  }
  HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);

  // Capture using PrintWindow with PW_RENDERFULLCONTENT.
  BOOL printResult = PrintWindow(hwnd, hdcMem, PW_RENDERFULLCONTENT);
  if (!printResult)
  {
    qDebug() << "PrintWindow failed for hwnd:" << hwnd;
    SelectObject(hdcMem, hOld);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);
    return QPixmap();
  }
  SelectObject(hdcMem, hOld);

  // Prepare to extract pixel data.
  BITMAP bmp;
  if (!GetObject(hBitmap, sizeof(BITMAP), &bmp))
  {
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);
    return QPixmap();
  }

  BITMAPINFOHEADER bi;
  ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = bmp.bmWidth;
  // Use a negative height to create a top-down DIB.
  bi.biHeight = -bmp.bmHeight;
  bi.biPlanes = 1;
  bi.biBitCount = 32; // Force 32-bit
  bi.biCompression = BI_RGB;
  int rowBytes = bmp.bmWidth * 4;
  int imageSize = rowBytes * bmp.bmHeight;

  // Allocate a buffer for pixel data.
  QByteArray buffer;
  buffer.resize(imageSize);
  HDC hdcScreen = GetDC(NULL);
  if (0 == GetDIBits(hdcScreen, hBitmap, 0, bmp.bmHeight, buffer.data(),
                     reinterpret_cast<BITMAPINFO *>(&bi), DIB_RGB_COLORS))
  {
    ReleaseDC(NULL, hdcScreen);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);
    return QPixmap();
  }
  ReleaseDC(NULL, hdcScreen);

  // Check if the capture is completely black.
  bool allBlack = true;
  const uchar *dataPtr = reinterpret_cast<const uchar *>(buffer.constData());
  int pixelCount = bmp.bmWidth * bmp.bmHeight;
  for (int i = 0; i < pixelCount; ++i)
  {
    int index = i * 4;
    // Check Blue, Green, and Red channels.
    if (dataPtr[index] != 0 || dataPtr[index + 1] != 0 ||
        dataPtr[index + 2] != 0)
    {
      allBlack = false;
      break;
    }
  }
  if (allBlack)
  {
    qDebug() << "Captured window (hwnd:" << hwnd
             << ") is completely black. Skipping.";
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);
    return QPixmap();
  }

  // Create a QImage from the pixel data.
  QImage image(reinterpret_cast<const uchar *>(buffer.constData()), bmp.bmWidth,
               bmp.bmHeight, rowBytes, QImage::Format_ARGB32);
  // Make a deep copy to detach from the temporary buffer.
  QPixmap pixmap = QPixmap::fromImage(image.copy());

  // Cleanup.
  DeleteObject(hBitmap);
  DeleteDC(hdcMem);
  ReleaseDC(hwnd, hdcWindow);
  return pixmap;
}

// Callback used with EnumWindows. For each window, try capturing a QPixmap.
BOOL CALLBACK EnumWindowsGrabProc(HWND hwnd, LPARAM lParam)
{
  GrabData *data = reinterpret_cast<GrabData *>(lParam);
  QPixmap pixmap = captureWindowAsPixmap(hwnd);
  if (!pixmap.isNull())
  {
    data->pixmaps->append(pixmap);
    data->names->append(std::to_string(reinterpret_cast<uintptr_t>(hwnd)));
    qDebug() << "Captured a non-black pixmap from window:" << hwnd;
  }
  return TRUE; // Continue enumeration.
}
#endif // Q_OS_WIN

#ifdef Q_OS_MAC
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>

// Helper function to convert a CGImageRef into a QImage.
static QImage QImageFromCGImage(CGImageRef imageRef)
{
  if (!imageRef)
    return QImage();

  size_t width = CGImageGetWidth(imageRef);
  size_t height = CGImageGetHeight(imageRef);
  // Create a QImage with the same dimensions in ARGB32 format.
  QImage image(width, height, QImage::Format_ARGB32);

  // Create a color space and bitmap context that writes directly into the
  // QImage data.
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef context = CGBitmapContextCreate(image.bits(), width, height,
                                               8, // bits per component
                                               image.bytesPerLine(), colorSpace,
                                               kCGImageAlphaPremultipliedFirst |
                                                   kCGBitmapByteOrder32Little);
  CGColorSpaceRelease(colorSpace);

  if (!context)
    return QImage();

  // Draw the CGImage into the context (which writes into QImage::bits()).
  CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
  CGContextRelease(context);

  return image;
}

// Helper function that captures a window (by its CGWindowID) and returns a
// QPixmap.
static QPixmap captureWindowAsPixmap(CGWindowID windowID)
{
  // Capture the window image. Using CGRectNull captures the entire window.
  CGImageRef imageRef =
      CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow,
                              windowID, kCGWindowImageDefault);
  if (!imageRef)
    return QPixmap();

  QImage image = QImageFromCGImage(imageRef);
  CGImageRelease(imageRef);

  if (image.isNull())
    return QPixmap();

  // Optionally, check if the image is completely black.
  bool allBlack = true;
  for (int y = 0; y < image.height(); ++y)
  {
    const QRgb *line = reinterpret_cast<const QRgb *>(image.scanLine(y));
    for (int x = 0; x < image.width(); ++x)
    {
      QRgb pixel = line[x];
      if (qRed(pixel) != 0 || qGreen(pixel) != 0 || qBlue(pixel) != 0)
      {
        allBlack = false;
        break;
      }
    }
    if (!allBlack)
      break;
  }
  if (allBlack)
  {
    qDebug() << "Captured window (ID:" << windowID
             << ") is completely black. Skipping.";
    return QPixmap();
  }

  return QPixmap::fromImage(image);
}
#endif // Q_OS_MAC

// Now implement grab_pixmaps_of_active_windows
std::pair<QList<QPixmap>, QList<std::string>>
WindowManager::grab_pixmaps_of_active_windows()
{
  QList<QPixmap> pixmaps;
  QList<std::string> names;
#ifdef Q_OS_WIN
  GrabData data;
  data.pixmaps = &pixmaps;
  data.names = &names;
  EnumWindows(EnumWindowsGrabProc, reinterpret_cast<LPARAM>(&data));
#else
  // Retrieve on-screen windows while excluding desktop elements.
  CFArrayRef windowList = CGWindowListCopyWindowInfo(
      kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
      kCGNullWindowID);
  if (windowList)
  {
    CFIndex count = CFArrayGetCount(windowList);
    for (CFIndex i = 0; i < count; i++)
    {
      CFDictionaryRef windowInfo =
          static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windowList, i));
      // Get the window's unique ID.
      CFNumberRef windowNumberRef = static_cast<CFNumberRef>(
          CFDictionaryGetValue(windowInfo, kCGWindowNumber));
      if (windowNumberRef)
      {
        int windowID;
        CFNumberGetValue(windowNumberRef, kCFNumberIntType, &windowID);
        QPixmap pixmap =
            captureWindowAsPixmap(static_cast<CGWindowID>(windowID));
        if (!pixmap.isNull())
        {
          pixmaps.append(pixmap);
          names.append(std::to_string(windowID));
          qDebug() << "Captured a non-black pixmap from window:" << windowID;
        }
      }
    }
    CFRelease(windowList);
  }
#endif
  return {pixmaps, names};
}
