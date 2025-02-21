#include "WindowManager.h"
#include <QDebug>
#include <QString>
#include <QStringList>

#ifdef Q_OS_WIN
#include <TlHelp32.h>
#include <stdio.h>
#include <windows.h>
#endif

#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#endif

// --- Windows Helper Code ---

#ifdef Q_OS_WIN

// Structure used to find the main window for a process.
struct MainWindowFinderData
{
  DWORD processId;
  HWND mainWindow;
};

// Callback used with EnumWindows to find the main window for a given process.
BOOL CALLBACK EnumWindowsMainWindowProc(HWND hwnd, LPARAM lParam)
{
  MainWindowFinderData *data = reinterpret_cast<MainWindowFinderData *>(lParam);
  DWORD wndProcessId = 0;
  GetWindowThreadProcessId(hwnd, &wndProcessId);
  if (wndProcessId != data->processId)
    return TRUE; // Not this process.

  // Heuristic: the main window is one with no owner.
  if (GetWindow(hwnd, GW_OWNER) != nullptr)
    return TRUE;

  data->mainWindow = hwnd;
  return FALSE; // Found it.
}

HWND GetMainWindowForProcess(DWORD processId)
{
  MainWindowFinderData data;
  data.processId = processId;
  data.mainWindow = nullptr;
  EnumWindows(EnumWindowsMainWindowProc, reinterpret_cast<LPARAM>(&data));
  return data.mainWindow;
}

// Helper function to save an HBITMAP as a BMP file using native Windows API.
BOOL SaveHBITMAPToBMPFile(HBITMAP hBitmap, const std::wstring &filename)
{
  BITMAP bmp;
  if (!GetObject(hBitmap, sizeof(BITMAP), &bmp))
    return FALSE;

  // Set up the BITMAPINFOHEADER.
  BITMAPINFOHEADER bi;
  ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = bmp.bmWidth;
  bi.biHeight = bmp.bmHeight;
  bi.biPlanes = 1;
  bi.biBitCount = 32; // Force 32-bit
  bi.biCompression = BI_RGB;
  bi.biSizeImage = 0; // For BI_RGB can be 0

  // Calculate the size of the image data.
  int rowSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4;
  DWORD dwBmpSize = rowSize * bmp.bmHeight;

  // Allocate memory for the pixel data.
  char *lpPixels = new char[dwBmpSize];
  if (!lpPixels)
    return FALSE;

  HDC hdcScreen = GetDC(NULL);
  BITMAPINFO biInfo;
  ZeroMemory(&biInfo, sizeof(BITMAPINFO));
  biInfo.bmiHeader = bi;

  // Retrieve the pixel data.
  if (0 == GetDIBits(hdcScreen, hBitmap, 0, bmp.bmHeight, lpPixels, &biInfo,
                     DIB_RGB_COLORS))
  {
    delete[] lpPixels;
    ReleaseDC(NULL, hdcScreen);
    return FALSE;
  }
  ReleaseDC(NULL, hdcScreen);

  // Check if the entire bitmap is black.
  // Each pixel is 4 bytes: Blue, Green, Red, Alpha.
  bool isBlack = true;
  for (DWORD i = 0; i < dwBmpSize; i += 4)
  {
    BYTE blue = lpPixels[i];
    BYTE green = lpPixels[i + 1];
    BYTE red = lpPixels[i + 2];
    if (blue != 0 || green != 0 || red != 0)
    {
      isBlack = false;
      break;
    }
  }
  if (isBlack)
  {
    qDebug()
        << "The captured bitmap is completely black. Skipping saving for file:"
        << QString::fromStdWString(filename);
    delete[] lpPixels;
    return FALSE;
  }

  // Set up the BMP file header.
  BITMAPFILEHEADER bmfHeader;
  bmfHeader.bfType = 0x4D42; // "BM"
  bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  bmfHeader.bfSize = bmfHeader.bfOffBits + dwBmpSize;
  bmfHeader.bfReserved1 = 0;
  bmfHeader.bfReserved2 = 0;

  // Write the BMP file.
  HANDLE hFile = CreateFile(filename.c_str(), GENERIC_WRITE, 0, NULL,
                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    delete[] lpPixels;
    return FALSE;
  }
  DWORD dwWritten = 0;
  WriteFile(hFile, &bmfHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
  WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
  WriteFile(hFile, lpPixels, dwBmpSize, &dwWritten, NULL);
  CloseHandle(hFile);
  delete[] lpPixels;
  return TRUE;
}

BOOL CaptureWindowToBMPFile(HWND hwnd, const std::wstring &filename)
{
  RECT rect;
  if (!GetWindowRect(hwnd, &rect))
  {
    qDebug() << "GetWindowRect failed for hwnd:" << hwnd;
    return FALSE;
  }
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;
  if (width <= 0 || height <= 0)
  {
    qDebug() << "Invalid window dimensions for hwnd:" << hwnd;
    return FALSE;
  }

  // Get the window's device context.
  HDC hdcWindow = GetDC(hwnd);
  if (!hdcWindow)
  {
    qDebug() << "GetDC failed for hwnd:" << hwnd;
    return FALSE;
  }

  // Create a compatible memory DC and bitmap.
  HDC hdcMem = CreateCompatibleDC(hdcWindow);
  HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, width, height);
  if (!hBitmap)
  {
    qDebug() << "CreateCompatibleBitmap failed for hwnd:" << hwnd;
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);
    return FALSE;
  }
  HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);

  // Use only PrintWindow with PW_RENDERFULLCONTENT.
  BOOL printResult = PrintWindow(hwnd, hdcMem, PW_RENDERFULLCONTENT);
  if (!printResult)
  {
    qDebug() << "PrintWindow failed for hwnd:" << hwnd;
    SelectObject(hdcMem, hOld);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);
    return FALSE;
  }

  SelectObject(hdcMem, hOld);

  // Save the captured bitmap as a BMP file.
  BOOL result = SaveHBITMAPToBMPFile(hBitmap, filename);
  if (!result)
  {
    qDebug() << "Failed to save BMP file for hwnd:" << hwnd;
  }

  // Cleanup.
  DeleteObject(hBitmap);
  DeleteDC(hdcMem);
  ReleaseDC(hwnd, hdcWindow);
  return result;
}

#endif // Q_OS_WIN

// A simple demonstration method.
void WindowManager::hello()
{
  qDebug() << "Hello, World from WindowManager!";
}
// Enumerate processes, print process names, and capture the main window (if
// found) using native API.
void WindowManager::enumerateProcessesAndCaptureMainWindow()
{
#ifdef Q_OS_WIN
  // Create a snapshot of all processes.
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE)
  {
    qDebug() << "Failed to create process snapshot.";
    return;
  }

  PROCESSENTRY32 pe;
  pe.dwSize = sizeof(PROCESSENTRY32);
  if (Process32First(snapshot, &pe))
  {
    do
    {
      DWORD processId = pe.th32ProcessID;
      QString processName = QString::fromWCharArray(pe.szExeFile);
      HWND mainWindow = GetMainWindowForProcess(processId);
      qDebug() << "Process:" << processName << "PID:" << processId
               << "MainWindow:" << mainWindow;
      if (mainWindow)
      {
        QString filename = QString("%1_%2.bmp").arg(processName).arg(processId);
        std::wstring wfilename = filename.toStdWString();
        if (CaptureWindowToBMPFile(mainWindow, wfilename))
          qDebug() << "Captured screenshot for process:" << processName;
        else
          qDebug() << "Failed to capture screenshot for process:"
                   << processName;
      }
    } while (Process32Next(snapshot, &pe));
  }
  else
  {
    qDebug() << "Failed to retrieve first process.";
  }
  CloseHandle(snapshot);
#elif defined(Q_OS_MAC)
  qDebug() << "enumerateProcessesAndCaptureMainWindow() not implemented for "
              "macOS yet.";
#else
  qDebug() << "enumerateProcessesAndCaptureMainWindow() not implemented on "
              "this platform.";
#endif
}
