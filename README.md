## Overview

This project is a Windows-based C application that simulates interaction between **User Mode** and **Kernel Mode** processes using **Interprocess Communication (IPC)** via **Named Pipes**.

It models how operating systems handle system calls: a user process sends requests, a kernel process interprets them, executes the requested service using a **function pointer jump table**, and returns the result.

The project reinforces:

- Operating Systems fundamentals  
- User vs Kernel mode separation  
- IPC using Named Pipes  
- Function pointers and dispatch tables  
- Client-server architecture  
- System call design  

---

## Features

### Kernel Process

- Creates and manages a named pipe server
- Waits for and handles client connections
- Parses syscall requests
- Dispatches calls using a jump table
- Sends responses back to user process
- Handles disconnects and shutdown gracefully

### User Process

- Connects to kernel via named pipe
- Menu-driven interface for syscall selection
- Sends formatted requests to kernel
- Receives and prints responses
- Supports repeated interactions until exit

### Simulated System Calls

- Get system time
- Generate random number
- Square a number
- Convert string to uppercase

---

## How It Works

1. Kernel process starts and creates a named pipe
2. User process connects to the pipe
3. User selects a syscall option
4. Request is sent to kernel
5. Kernel parses function code
6. Jump table dispatches correct function
7. Result is computed and returned
8. User displays response
9. Repeat until exit

---

## System Call Table

| Code |          Operation          |
|------|-----------------------------|
| 1    | Get System Time             |
| 2    | Get Random Number           |
| 3    | Square a Number             |
| 4    | Convert String to Uppercase |
| 0    | Shutdown System             |

---

## Project Structure

### `kernel.c`

- Implements kernel-mode process
- Handles IPC server (Named Pipe)
- Uses function pointer jump table
- Executes syscall handlers

### `user.c`

- Implements user-mode process
- Connects to kernel pipe
- Sends syscall requests
- Displays kernel responses

---

## IPC Mechanism

This project uses Windows Named Pipes
This enables: 
- Bidirectional communication
- Process isolation
- Reliable message passing
- Real-world IPC simulation

---

## Author

Brady Mills
