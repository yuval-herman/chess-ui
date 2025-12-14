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

bool pipe_is_connected() {
  return g_hPipe != INVALID_HANDLE_VALUE;
}

bool pipe_init() {
  _tprintf(TEXT("Attempting to create named pipe server: %s\n"), PIPE_NAME);

  // Create a named pipe instance
  g_hPipe = CreateNamedPipe(
      PIPE_NAME,                  // Pipe name
      PIPE_ACCESS_DUPLEX,         // Read/write access
      PIPE_TYPE_MESSAGE |         // Message type pipe
          PIPE_READMODE_MESSAGE | // Message-read mode
          PIPE_WAIT,              // Blocking mode
      PIPE_UNLIMITED_INSTANCES,   // Max instances (we only expect one)
      BUFFER_SIZE,                // Output buffer size
      BUFFER_SIZE,                // Input buffer size
      TIMEOUT_MS,                 // Default timeout
      NULL);                      // Default security attributes

  if (g_hPipe == INVALID_HANDLE_VALUE) {
    _tprintf(TEXT("Error creating pipe: 0x%08lx\n"), GetLastError());
    return false;
  }

  _tprintf(TEXT("Pipe created. Waiting for client connection...\n"));

  // Block until a client connects to the pipe
  if (ConnectNamedPipe(g_hPipe, NULL) != FALSE) {
    _tprintf(TEXT("Client connected successfully!\n"));
  } else {
    // The client may have connected between CreateNamedPipe and
    // ConnectNamedPipe If ConnectNamedPipe fails with ERROR_PIPE_CONNECTED, it
    // means the client is already there.
    if (GetLastError() == ERROR_PIPE_CONNECTED) {
      _tprintf(TEXT("Client already connected.\n"));
    } else {
      _tprintf(TEXT("Error connecting client: 0x%08lx\n"), GetLastError());
      CloseHandle(g_hPipe);
      g_hPipe = INVALID_HANDLE_VALUE;
      return false;
    }
  }
  return true;
}

bool pipe_send_message(char *msg) {
  if (g_hPipe == INVALID_HANDLE_VALUE) {
    _tprintf(TEXT("Pipe not initialized or connected.\n"));
    return false;
  }

  DWORD cbToWrite = (DWORD)strlen(msg) + 1; // +1 for the null terminator
  DWORD cbWritten;
  BOOL bResult;

  _tprintf(TEXT("Sending message: \"%s\" (%lu bytes)\n"), (LPTSTR)msg,
           cbToWrite);

  bResult = WriteFile(g_hPipe,    // Handle to pipe
                      msg,        // Message to send
                      cbToWrite,  // Length of message
                      &cbWritten, // Number of bytes written
                      NULL);      // Not overlapped

  if (!bResult || cbToWrite != cbWritten) {
    _tprintf(TEXT("WriteFile failed w/err 0x%08lx\n"), GetLastError());
    return false;
  }

  _tprintf(TEXT("Sent %lu bytes.\n"), cbWritten);
  return true;
}

char *pipe_get_message() {
  if (g_hPipe == INVALID_HANDLE_VALUE) {
    _tprintf(TEXT("Pipe not initialized or connected.\n"));
    return NULL;
  }

  DWORD cbRead;
  BOOL bResult;

  _tprintf(TEXT("Waiting to receive message...\n"));

  bResult = ReadFile(g_hPipe,     // Handle to pipe
                     chReply,     // Buffer to receive the data
                     BUFFER_SIZE, // Size of buffer
                     &cbRead,     // Number of bytes read
                     NULL);       // Not overlapped

  if (bResult) {
    // Successfully read data. The message should be null-terminated by
    // the client, but we ensure it just in case.
    chReply[cbRead] = '\0';
    _tprintf(TEXT("Received %lu bytes. Message: \"%s\"\n"), cbRead,
             (LPTSTR)chReply);

    return chReply;
  } else {
    // ReadFile failed
    _tprintf(TEXT("ReadFile failed w/err 0x%08lx\n"), GetLastError());
    return NULL;
  }
}

bool pipe_has_new_message() {
  DWORD bytesAvail = 0;
  if(!PeekNamedPipe(g_hPipe, NULL, 0, NULL, &bytesAvail, NULL)) {
    printf("Error when trying to peek into pipe\n");
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
    _tprintf(TEXT("Pipe closed.\n"));
  }
}

// int main() {
//   if (!pipe_init()) {
//     printf("Failed creating named pipe, exiting...");
//     exit(1);
//   }

//   char *received_msg = pipe_get_message();
//   if (received_msg != NULL) {
//     // Process the message
//     _tprintf(TEXT("\nServer received: %s\n"), (LPTSTR)received_msg);

//     // Send a response
//     char server_response[] = "quit";
//     pipe_send_message(server_response);
//   }

//   pipe_close();
//   return 0;
// }
