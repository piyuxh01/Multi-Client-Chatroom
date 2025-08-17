# Multi-Client Chatroom Application

![C++](https://img.shields.io/badge/Language-C++-blue)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey)
![License](https://img.shields.io/badge/License-MIT-green)

A **C++ chatroom** using **socket programming** and **multi-threading**. It supports multiple clients chatting simultaneously, with **color-coded usernames, timestamps, emojis, and system notifications**.  

---

## Features

- Multi-client support  
- Color-coded usernames  
- Timestamps on messages  
- Emoji support (UTF-8)  
- System notifications when users **join** or **leave**  
- Cross-platform: Windows & Linux  

---

## Screenshot

![Chatroom Screenshot](screenshot.png)  

---

## Quick Start

| Step | Windows (MinGW) | Linux / macOS |
|------|-----------------|---------------|
| **1. Compile Server** | `g++ server.cpp -o server.exe -lws2_32` | `g++ server.cpp -lpthread -o server` |
| **2. Compile Client** | `g++ client.cpp -o client.exe -lws2_32` | `g++ client.cpp -lpthread -o client` |
| **3. Run Server** | `server.exe` | `./server` |
| **4. Run Client** | `client.exe` | `./client` |
| **5. Run Multiple Clients** | Open new terminal, run `client.exe` again | Open new terminal, run `./client` again |
| **6. Exit Client** | Type `#exit` or press **Ctrl+C** | Type `#exit` or press **Ctrl+C** |

---

## How It Works

1. **Server** listens on **port 10000**.  
2. Each **client** chooses a **username** when connecting.  
3. Messages are **broadcasted** to all clients in **real-time**.  
4. Each user has a **consistent color** for their username.  
5. **Timestamps** are automatically added to messages.  
6. **Emojis** can be sent if console supports UTF-8.  
7. System messages announce **joins and leaves**.  

---

## Example Session

### Server Terminal

