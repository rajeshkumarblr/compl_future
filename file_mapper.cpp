#include "BoundedQueue.h"
#include "print_utils.h"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <directory_path>" << endl;
    return 1;
  }

  string rootPath = argv[1];
  if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
    cerr << "Error: " << rootPath << " is not a valid directory." << endl;
    return 1;
  }

  vector<string> filePaths;
  cout << "Scanning directory: " << rootPath << "..." << endl;

  try {
    for (const auto &entry : fs::recursive_directory_iterator(rootPath)) {
      if (entry.is_regular_file()) {
        filePaths.push_back(entry.path().string());
      }
    }
  } catch (const fs::filesystem_error &e) {
    cerr << "Error during traversal: " << e.what() << endl;
    return 1;
  }

  cout << "Found " << filePaths.size() << " files." << endl;
  if (!filePaths.empty()) {
    cout << "Files: " << filePaths << endl;
  }

  return 0;
}