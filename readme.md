# 🏛️ University Laboratory Management Server

![C](https://img.shields.io/badge/Language-C-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20POSIX-lightgrey.svg)
![Concurrency](https://img.shields.io/badge/Concurrency-pthreads-orange.svg)


A highly concurrent, thread-safe server application built entirely in **C** to manage university laboratory resources. 

This project simulates a real-world Operating System bottleneck: allocating limited physical equipment (like PCs, oscilloscopes, or testing kits) to a massive queue of students without causing race conditions, memory corruption, or CPU busy-waiting.

## ✨ Features

* **⚡ Thread Pool Architecture:** Pre-spawns a pool of background worker threads. Uses `pthread_cond_t` (Condition Variables) to ensure threads consume **0% CPU** while idle.
* **🔒 Strict Memory Safety:** Implements POSIX Mutexes (`pthread_mutex_t`) to completely prevent race conditions during concurrent data reads/writes.
* **🚦 Real-Time Hardware Mapping:** Uses POSIX Semaphores (`sem_t`) as "digital bouncers" to strictly enforce physical equipment limits. 
* **🔄 Circular Request Queue:** Achieves `O(1)` time complexity for enqueueing and dequeueing student requests.
* **📊 Live OS Monitor:** A dynamic, refreshing terminal UI that tracks active threads and live equipment availability.
* **💾 Data Persistence & CSV Logging:** 
  * Saves core state to `labs.txt` and `students.txt`.
  * Automatically generates `allocation.csv` and `attendance.csv` for enterprise-style tracking.
* **🛑 Graceful Shutdown:** Traps OS signals (`SIGINT` / Ctrl+C) to wake sleeping threads, destroy mutexes/semaphores, and prevent memory leaks.

## 🛠️ Tech Stack

* **Language:** C (Standard Enterprise Modularity)
* **Compiler:** GCC
* **OS Primitives:** POSIX Threads (`<pthread.h>`), Semaphores (`<semaphore.h>`), Signals (`<signal.h>`)
* **Environment:** Linux / Ubuntu / WSL

## 📂 File Structure

```text
├── lab_system.h        # System blueprints, Structs, and Extern variables
├── main.c              # Entry point, thread initialization, and signal trapping
├── admin.c             # Dynamic UI, Data entry, and CSV/TXT File I/O
├── worker.c            # Pure concurrency logic, queue popping, and semaphore locks
└── README.md           # You are here
