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

vector<string> getFiles(const string &path) {
  vector<string> filePaths;
  try {
    if (fs::exists(path) && fs::is_directory(path)) {
      for (const auto &entry : fs::recursive_directory_iterator(path)) {
        if (entry.is_regular_file()) {
          filePaths.push_back(entry.path().string());
        }
      }
    }
  } catch (const fs::filesystem_error &e) {
    cerr << "Error during traversal: " << e.what() << endl;
  }
  return filePaths;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <directory_path>" << endl;
    return 1;
  }

  string rootPath = argv[1];
  cout << "Scanning directory: " << rootPath << "..." << endl;

  vector<string> filePaths = getFiles(rootPath);

  cout << "Found " << filePaths.size() << " files." << endl;
  if (!filePaths.empty()) {
    cout << "Files: " << filePaths << endl;
  }

  return 0;
}