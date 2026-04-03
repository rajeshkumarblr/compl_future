# Future File Processor & Concurrency Lab

A collection of high-performance asynchronous and concurrency patterns in C++23.

## Overview

This project demonstrates various concurrency patterns in modern C++, ranging from asynchronous file processing to thread-safe data structures and producer-consumer implementations.

## Implementations

### 1. Asynchronous File Processing
- **`future.cpp`**: Uses `std::async` with a sliding window pattern to process file metadata.
- **`thread_pool.cpp`**: Custom thread pool implementation for high-throughput file processing.

### 2. Producer-Consumer Patterns
- **`my_prod_cons.cpp`**: A decoupled producer-consumer implementation where production and consumption logic are passed as lambdas.
- **`bounded_queue.cpp`**: A generic, thread-safe `BoundedQueue<T>` implementation using templates and `std::optional` for clean termination.
- **`async_producer.cpp`**: A simple example of using `std::async` to return a container from a task.

## Build and Run

### Requirements
- **Compiler**: `g++-14` or later (for C++23 support).
- **Standard**: C++23.

### Compiling
Run the following command to build all targets:
```bash
make
```

### Running
Each target can be run independently:
```bash
./future [path] [concurrency]
./thread_pool [path] [threads]
./my_prod_cons
./bounded_queue
./async_producer
```

## Project Structure
- `Makefile`: Centralized build system for all components.
- `.gitignore`: Configured to ignore binary artifacts.
- `*.cpp`: Individual concurrency pattern implementations.
