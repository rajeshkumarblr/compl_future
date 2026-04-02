# Future File Processor

A high-performance asynchronous file metadata processor in C++20 using `std::future` and `std::async`.

## Overview

This project demonstrates how to efficiently process a large number of files by leveraging C++ concurrency features. It traverses a directory recursively, collects file metadata (filenames and sizes), and prints the results.

## Design Highlights

### 1. Asynchronous Task Management (Sliding Window)
To avoid overwhelming the system with thousands of concurrent threads, the project implements a **Sliding Window** pattern:
- A `std::deque` of `std::future` objects tracks active tasks.
- The `processBatch` function ensures that only a specified number of futures (`maxFuturesInFlight`) are active at any given time.
- When the limit is reached, it waits for the oldest task to complete before launching a new one.

### 2. Thread-Safe Result Collection
Results are collected in a shared `std::vector<MetaData>`. To prevent data races, a `std::mutex` and `std::lock_guard` are used to synchronize access during result insertion.

### 3. Customizable Print Strategy
The project uses `std::function` to define a customizable print behavior. This allows the logger to be swapped or modified at runtime without changing the core logic:
```cpp
std::function<void(const std::string&)> print = [](const std::string& msg) {
    std::cout << "[INFO] " << msg << std::endl;
};
```

## Prerequisites

- C++20 compatible compiler (e.g., GCC 10+, Clang 10+)
- Make

## Build and Run

### Compiling
Run the following command to build the project:
```bash
make
```

### Running
Execute the binary:
```bash
./future [path] [concurrency_limit]
```
- `path`: The directory to process (default: `/home/rajesh/proj/dlake`).
- `concurrency_limit`: Maximum number of concurrent tasks (default: 10).

Example:
```bash
./future /home/rajesh/proj/future 5
```

## Project Structure

- `future.cpp`: Core logic and entry point.
- `Makefile`: Build configuration.
- `MetaData`: Data structure for file information.
- `processBatch`: Generic sliding window implementation for future management.
