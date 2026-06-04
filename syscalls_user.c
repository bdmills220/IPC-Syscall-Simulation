/*
* Developer:           Brady Mills
* Project:             Syscall Simulation Using Interprocess Communication (IPC) - User Mode
* Last Updated Date:   3/23/25
* Description:
*       This part of the program is the User Process (Simulated Syscall Request Sender).
*           1. Prompts the user to select a function code
*           2. Takes input parameters if needed
*           3. Sends a formatted request to the kernel process
*           4. Receives and prints the kernel’s response
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PIPE_NAME TEXT("\\\\.\\pipe\\millsb_A2_syscall_pipe")  // Define the name of the named pipe
#define BUFFER_SIZE 1024  // Define the buffer size for messages

int main() {
    HANDLE hPipe;  // Handle for the named pipe
    char buffer[BUFFER_SIZE] = { 0 };  // Buffer for reading/writing messages
    DWORD dwWritten, dwRead;  // Variables to store the number of bytes written/read

    // Add a delay to ensure the kernel process is ready
    printf("Waiting for kernel process to initialize...\n");
    Sleep(2000);  // Delay for 2 seconds (2000 milliseconds)

    printf("Attempting to connect to named pipe...\n");

    // Loop to attempt connecting to the named pipe
    while (1) {
        hPipe = CreateFile(
            PIPE_NAME,  // Name of the pipe
            GENERIC_READ | GENERIC_WRITE,  // Read/write access
            0,  // No sharing
            NULL,  // Default security attributes
            OPEN_EXISTING,  // Open existing pipe
            0,  // Default attributes
            NULL  // No template file
        );

        if (hPipe != INVALID_HANDLE_VALUE) {
            break;  // Successfully connected
        }

        DWORD dwError = GetLastError();
        if (dwError == ERROR_PIPE_BUSY) {
            printf("Pipe is busy, retrying...\n");
            Sleep(500);  // Wait before retrying
        }
        else {
            printf("Failed to open named pipe. Error: %d\n", dwError);
            return 1;
        }
    }

    printf("Connected to kernel process.\n");

    // Main loop to handle user input and communication with the kernel process
    while (1) {
        int functionCode;  // Variable to store the function code
        char param[BUFFER_SIZE] = { 0 };  // Variable to store the parameter (if any)

        // Prompt the user to select a function code
        printf("\nEnter function code (1-4, 0 to exit):\n");
        printf("1. Get System Time\n");
        printf("2. Get Random Number\n");
        printf("3. Square a Number\n");
        printf("4. Convert String to Uppercase\n");
        printf("0. Exit (Shutdown User and Kernel)\n");

        // Read the function code from the user
        if (scanf_s("%d", &functionCode) != 1) {
            printf("Error: Invalid input.\n");
            continue;
        }

        // If the user chooses to exit, send the exit request and break the loop
        if (functionCode == 0) {
            sprintf_s(buffer, BUFFER_SIZE, "%d", functionCode);  // Send function code 0 to kernel
            WriteFile(hPipe, buffer, (DWORD)strlen(buffer) + 1, &dwWritten, NULL);
            printf("Exiting user process...\n");
            break;
        }

        // If the function requires a parameter, prompt the user for it
        if (functionCode == 3 || functionCode == 4) {
            printf("Enter parameter: ");
            scanf_s(" %[^\n]", param, (unsigned)_countof(param));
        }

        // Format the request message with the function code and parameter
        sprintf_s(buffer, BUFFER_SIZE, "%d %s", functionCode, param);

        // Send the request to the kernel process
        if (!WriteFile(hPipe, buffer, (DWORD)strlen(buffer) + 1, &dwWritten, NULL)) {
            printf("Failed to write to pipe. Error: %d\n", GetLastError());
            break;
        }

        // Read the response from the kernel process
        if (ReadFile(hPipe, buffer, BUFFER_SIZE, &dwRead, NULL)) {
            buffer[dwRead] = '\0';  // Null-terminate the response
            printf("Response: %s\n", buffer);  // Print the response
        }
        else {
            printf("Failed to read from pipe. Error: %d\n", GetLastError());
            break;
        }
    }

    CloseHandle(hPipe);  // Close the pipe handle
    printf("User process has exited.\n");
    return 0;
}