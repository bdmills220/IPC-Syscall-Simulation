/*
* Developer:           Brady Mills
* Project:             Syscall Simulation Using Interprocess Communication (IPC) - Kernel Mode
* Last Updated Date:   3/23/25
* Description:
*       This part of the program is the Kernel Process (Simulated Syscall Handler).
*           1. Listens for requests from the user process
*           2. Extracts the function code from the message
*           3. Uses a jump table (function pointer array) to determine which function to execute
*           4. Processes the request and sends back a response
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PIPE_NAME TEXT("\\\\.\\pipe\\millsb_A2_syscall_pipe")  // Define the name of the named pipe
#define BUFFER_SIZE 1024  // Define the buffer size for messages

// Define the function pointer type for the jump table
typedef int (*PFI)(char*, char*);
PFI jump_table[4];  // Jump table with 4 entries (4 functions)

// Function prototypes for the syscall operations
int get_system_time(char* response, char* param);
int get_random_number(char* response, char* param);
int square_number(char* response, char* param);
int convert_to_uppercase(char* response, char* param);

// Kernel mode process
int main() {
    HANDLE hPipe;  // Handle for the named pipe
    char buffer[BUFFER_SIZE] = { 0 };  // Buffer for reading/writing messages
    DWORD dwRead, dwWritten;  // Variables to store the number of bytes read/written

    // Initialize the jump table with the functions
    jump_table[0] = get_system_time;
    jump_table[1] = get_random_number;
    jump_table[2] = square_number;
    jump_table[3] = convert_to_uppercase;

    printf("[Kernel Mode] Initializing named pipe...\n");

    // Create the named pipe
    hPipe = CreateNamedPipe(
        PIPE_NAME,  // Name of the pipe
        PIPE_ACCESS_DUPLEX,  // Pipe is bidirectional
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,  // Message mode and blocking
        1,  // Maximum number of instances
        BUFFER_SIZE,  // Output buffer size
        BUFFER_SIZE,  // Input buffer size
        0,  // Default timeout
        NULL  // Default security attributes
    );

    // Check if the pipe was created successfully
    if (hPipe == INVALID_HANDLE_VALUE) {
        printf("[ERROR] Failed to create pipe. Error Code: %d\n", GetLastError());
        return 1;
    }

    printf("[READY] Waiting for user process to connect...\n");

    // Main loop to handle client connections and requests
    while (1) {
        // Wait for a client to connect
        if (ConnectNamedPipe(hPipe, NULL)) {
            printf("[CONNECTED] User process is now connected.\n");

            // Inner loop to handle multiple requests from the connected client
            while (1) {
                memset(buffer, 0, sizeof(buffer));  // Clear the buffer

                // Read a request from the client
                if (!ReadFile(hPipe, buffer, BUFFER_SIZE - 1, &dwRead, NULL)) {
                    DWORD error = GetLastError();
                    if (error == ERROR_BROKEN_PIPE) {
                        printf("[INFO] User process disconnected.\n");
                    }
                    else {
                        printf("[ERROR] Failed to read from pipe. Error Code: %d\n", error);
                    }
                    break;
                }

                buffer[dwRead] = '\0';  // Null-terminate the received message
                printf("[REQUEST] %s\n", buffer);

                int functionCode;  // Variable to store the function code
                char param[BUFFER_SIZE] = { 0 };  // Variable to store the parameter (if any)
                char response[BUFFER_SIZE] = { 0 };  // Variable to store the response

                // Parse the function code and parameter from the request
                int numArgs = sscanf_s(buffer, "%d %[^\n]", &functionCode, param, (unsigned)_countof(param));

                if (numArgs < 1) {
                    // Invalid request format
                    sprintf_s(response, BUFFER_SIZE, "Error: Invalid request format.");
                }
                else if (functionCode == 0) {
                    // Exit request
                    sprintf_s(response, BUFFER_SIZE, "Shutting down kernel process...");
                    WriteFile(hPipe, response, (DWORD)strlen(response) + 1, &dwWritten, NULL);
                    printf("[SHUTDOWN] Shutting down kernel process...\n");
                    CloseHandle(hPipe);
                    return 0;  // Exit the kernel process
                }
                else if (functionCode >= 1 && functionCode <= 4) {
                    // Valid function code, call the corresponding function from the jump table
                    PFI return_code = jump_table[functionCode - 1];
                    return_code(response, param);  // Execute the function
                }
                else {
                    // Invalid function code
                    sprintf_s(response, BUFFER_SIZE, "Error: Invalid Function Code %d", functionCode);
                }

                printf("[RESPONSE] %s\n", response);

                // Send the response back to the client
                if (!WriteFile(hPipe, response, (DWORD)strlen(response) + 1, &dwWritten, NULL)) {
                    printf("[ERROR] Failed to send response. Error Code: %d\n", GetLastError());
                    break;
                }
            }

            printf("[DISCONNECTED] User process disconnected.\n");
            DisconnectNamedPipe(hPipe);  // Disconnect the client
        }
        else {
            printf("[ERROR] Failed to connect user process. Error Code: %d\n", GetLastError());
            break;
        }
    }

    CloseHandle(hPipe);  // Close the pipe handle
    printf("Kernel process has exited.\n");
    return 0;
}

// Function Definitions

// Function to get the current system time
int get_system_time(char* response, char* param) {
    SYSTEMTIME st;
    GetSystemTime(&st);  // Get the current system time
    sprintf_s(response, BUFFER_SIZE, "Success: System time is %02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
    return 0;  // Return 0 for success
}

// Function to generate a random number
int get_random_number(char* response, char* param) {
    srand((unsigned int)time(NULL));  // Seed the random number generator
    int randomNum = rand() % 100;  // Generate a random number between 0 and 99
    sprintf_s(response, BUFFER_SIZE, "Success: Random Number is %d", randomNum);
    return 0;  // Return 0 for success
}

// Function to square a number
int square_number(char* response, char* param) {
    int number;
    if (sscanf_s(param, "%d", &number) == 1) {  // Parse the input number
        sprintf_s(response, BUFFER_SIZE, "Success: %d", number * number);  // Square the number
        return 0;
    }
    else {
        sprintf_s(response, BUFFER_SIZE, "Error: Invalid number format.");  // Handle invalid input
        return 1;  // Return 1 for error
    }
}

// Function to convert a string to uppercase
int convert_to_uppercase(char* response, char* param) {
    char upperParam[BUFFER_SIZE] = { 0 };

    // Convert the string to uppercase
    for (int i = 0; param[i] != '\0'; i++) {
        upperParam[i] = (char)toupper(param[i]);
    }

    // Ensure the string is null-terminated
    upperParam[strlen(param)] = '\0';

    // Format the response
    sprintf_s(response, BUFFER_SIZE, "Success: %s", upperParam);
    return 0;  // Return 0 for success
}