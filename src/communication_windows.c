#include "communication.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <windows.h>

#define PIPE_NAME TEXT("\\\\.\\pipe\\chessPipe")
#define BUFFER_SIZE 1024
#define TIMEOUT_MS 5000

HANDLE g_hPipe = INVALID_HANDLE_VALUE;
char chReply[BUFFER_SIZE];
OVERLAPPED g_connectOverlapped = {0};
HANDLE g_hConnectEvent = NULL;
bool g_bConnected = false;

bool pipe_is_connected() {
  if (g_bConnected) {
    return true;
  }

  if (g_hPipe == INVALID_HANDLE_VALUE || g_hConnectEvent == NULL) {
    return false;
  }

  // Check if the connection has completed without blocking
  DWORD cbTransferred;
  BOOL bResult = GetOverlappedResult(g_hPipe, &g_connectOverlapped, &cbTransferred, FALSE);
  if (bResult) {
    _tprintf(TEXT("Client connected asynchronously!\n"));
    g_bConnected = true;
    return true;
  } else {
    DWORD dwErr = GetLastError();
    if (dwErr == ERROR_IO_INCOMPLETE) {
      // Still pending
      return false;
    } else {
      _tprintf(TEXT("Connection failed: 0x%08lx\n"), dwErr);
      pipe_close();
      return false;
    }
  }
}

bool pipe_init() {
  _tprintf(TEXT("Attempting to create named pipe server: %s\n"), PIPE_NAME);

  // Create a named pipe instance
  g_hPipe = CreateNamedPipe(
      PIPE_NAME,                        // Pipe name
      PIPE_ACCESS_DUPLEX |              // Read/write access
      FILE_FLAG_OVERLAPPED,             // Overlapped mode for async operations
      PIPE_TYPE_MESSAGE |               // Message type pipe
          PIPE_READMODE_MESSAGE |       // Message-read mode
          PIPE_WAIT,                    // Blocking mode for waits
      PIPE_UNLIMITED_INSTANCES,         // Max instances (we only expect one)
      BUFFER_SIZE,                      // Output buffer size
      BUFFER_SIZE,                      // Input buffer size
      TIMEOUT_MS,                       // Default timeout
      NULL);                            // Default security attributes

  if (g_hPipe == INVALID_HANDLE_VALUE) {
    _tprintf(TEXT("Error creating pipe: 0x%08lx\n"), GetLastError());
    return false;
  }

  _tprintf(TEXT("Pipe created. Initiating asynchronous client connection...\n"));

  // Set up overlapped structure for connect
  ZeroMemory(&g_connectOverlapped, sizeof(OVERLAPPED));
  g_hConnectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (g_hConnectEvent == NULL) {
    _tprintf(TEXT("Error creating event: 0x%08lx\n"), GetLastError());
    CloseHandle(g_hPipe);
    g_hPipe = INVALID_HANDLE_VALUE;
    return false;
  }
  g_connectOverlapped.hEvent = g_hConnectEvent;

  // Initiate asynchronous connect
  BOOL fSuccess = ConnectNamedPipe(g_hPipe, &g_connectOverlapped);
  if (fSuccess) {
    _tprintf(TEXT("Client connected immediately!\n"));
    g_bConnected = true;
  } else {
    DWORD dwErr = GetLastError();
    if (dwErr == ERROR_IO_PENDING) {
      _tprintf(TEXT("Connection request pending...\n"));
      // g_bConnected remains false until checked
    } else if (dwErr == ERROR_PIPE_CONNECTED) {
      _tprintf(TEXT("Client already connected.\n"));
      g_bConnected = true;
    } else {
      _tprintf(TEXT("Error initiating connection: 0x%08lx\n"), dwErr);
      CloseHandle(g_hConnectEvent);
      g_hConnectEvent = NULL;
      CloseHandle(g_hPipe);
      g_hPipe = INVALID_HANDLE_VALUE;
      return false;
    }
  }
  return true;
}

bool pipe_send_message(char *msg) {
  if (!pipe_is_connected()) {
    _tprintf(TEXT("Pipe not initialized or connected.\n"));
    return false;
  }

  DWORD cbToWrite = (DWORD)strlen(msg) + 1; // +1 for the null terminator
  DWORD cbWritten;
  BOOL bResult;

  _tprintf(TEXT("Sending message: \"%s\" (%lu bytes)\n"), (LPTSTR)msg, cbToWrite);

  // Set up local overlapped for write
  OVERLAPPED overlapped = {0};
  HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (hEvent == NULL) {
    _tprintf(TEXT("Error creating event for write: 0x%08lx\n"), GetLastError());
    return false;
  }
  overlapped.hEvent = hEvent;

  bResult = WriteFile(g_hPipe, msg, cbToWrite, &cbWritten, &overlapped);

  if (bResult) {
    // Completed immediately
    _tprintf(TEXT("Sent %lu bytes immediately.\n"), cbWritten);
  } else {
    DWORD dwErr = GetLastError();
    if (dwErr == ERROR_IO_PENDING) {
      // Wait for completion (blocking)
      bResult = GetOverlappedResult(g_hPipe, &overlapped, &cbWritten, TRUE);
      if (bResult) {
        _tprintf(TEXT("Sent %lu bytes.\n"), cbWritten);
      } else {
        _tprintf(TEXT("GetOverlappedResult failed for write: 0x%08lx\n"), GetLastError());
      }
    } else {
      _tprintf(TEXT("WriteFile failed: 0x%08lx\n"), dwErr);
    }
  }

  CloseHandle(hEvent);

  if (!bResult || cbToWrite != cbWritten) {
    return false;
  }
  return true;
}

char *pipe_get_message() {
  if (!pipe_is_connected()) {
    _tprintf(TEXT("Pipe not initialized or connected.\n"));
    return NULL;
  }

  DWORD cbRead;
  BOOL bResult;

  _tprintf(TEXT("Waiting to receive message...\n"));

  // Set up local overlapped for read
  OVERLAPPED overlapped = {0};
  HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (hEvent == NULL) {
    _tprintf(TEXT("Error creating event for read: 0x%08lx\n"), GetLastError());
    return NULL;
  }
  overlapped.hEvent = hEvent;

  bResult = ReadFile(g_hPipe, chReply, BUFFER_SIZE, &cbRead, &overlapped);

  if (bResult) {
    // Completed immediately
  } else {
    DWORD dwErr = GetLastError();
    if (dwErr == ERROR_IO_PENDING) {
      // Wait for completion (blocking)
      bResult = GetOverlappedResult(g_hPipe, &overlapped, &cbRead, TRUE);
    } else {
      _tprintf(TEXT("ReadFile failed: 0x%08lx\n"), dwErr);
      bResult = FALSE;
    }
  }

  CloseHandle(hEvent);

  if (bResult) {
    // Successfully read data. Ensure null-termination
    if (cbRead < BUFFER_SIZE) {
      chReply[cbRead] = '\0';
    } else {
      chReply[BUFFER_SIZE - 1] = '\0';
    }
    _tprintf(TEXT("Received %lu bytes. Message: \"%s\"\n"), cbRead, (LPTSTR)chReply);
    return chReply;
  } else {
    _tprintf(TEXT("GetOverlappedResult failed for read: 0x%08lx\n"), GetLastError());
    return NULL;
  }
}

bool pipe_has_new_message() {
  DWORD bytesAvail = 0;
  if (!PeekNamedPipe(g_hPipe, NULL, 0, NULL, &bytesAvail, NULL)) {
    _tprintf(TEXT("Error when trying to peek into pipe: 0x%08lx\n"), GetLastError());
    return false;
  }
  return bytesAvail > 0;
}

void pipe_close() {
  if (g_hPipe != INVALID_HANDLE_VALUE) {
    // Disconnects the pipe instance from the client
    DisconnectNamedPipe(g_hPipe);

    // Close the handle
    CloseHandle(g_hPipe);
    g_hPipe = INVALID_HANDLE_VALUE;
  }
  if (g_hConnectEvent != NULL) {
    CloseHandle(g_hConnectEvent);
    g_hConnectEvent = NULL;
  }
  g_bConnected = false;
  _tprintf(TEXT("Pipe closed.\n"));
}
