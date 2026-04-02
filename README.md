# Future File Processor

A high-performance asynchronous file metadata processor in C++20 using `std::future` and `std::async`.

## Overview

This project demonstrates how to efficiently process a large number of files by leveraging C++ concurrency features. It traverses a directory recursively, collects file metadata (filenames and sizes), and prints the results.

## Implementations

The project provides two different implementations for asynchronous file processing:

### 1. Future-based Implementation (`future.cpp`)
- Uses `std::async` to launch tasks.
- Implements a **Sliding Window** pattern to limit concurrency manually.
- Simpler implementation but less control over the underlying thread pool (handles task scheduling via the OS/Standard Library).

### 2. Thread Pool Implementation (`thread_pool.cpp`)
- Implements a custom **ThreadPool** class.
- Uses a fixed number of worker threads and a task queue.
- More efficient for high-throughput tasks as it reuses threads rather than creating/destroying them.
- Provides better control over system resource usage.

## Build and Run

### Compiling
Run the following command to build both versions:
```bash
make
```

### Running

To run the future version:
```bash
./future [path] [concurrency_limit]
```

To run the thread pool version:
```bash
./thread_pool [path] [thread_count]
```

Example:
```bash
./thread_pool /home/rajesh/proj/future 5
```

## Project Structure

- `future.cpp`: Core logic and entry point.
- `Makefile`: Build configuration.
- `MetaData`: Data structure for file information.
- `processBatch`: Generic sliding window implementation for future management.
