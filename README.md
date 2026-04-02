# compl_future

A C++20 project demonstrating asynchronous batch processing of files with concurrency control using `std::future` and `std::async`.

## Features
- Recursive directory traversal.
- Asynchronous file metadata (size) retrieval.
- Configurable concurrency limit via command-line arguments.
- Real-time progress display.

## Build
```bash
make
```

## Usage
```bash
./future [path] [concurrency]
```
- `path`: The root directory to scan (default: `/home/rajesh`).
- `concurrency`: The maximum number of concurrent operations (default: `10`).

## Example
```bash
./future /home/rajesh/proj 20
```
