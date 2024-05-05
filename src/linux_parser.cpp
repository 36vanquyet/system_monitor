#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>

#include "linux_parser.h"

#define UTIME_INDEX 14
#define CSTIME_INDEX 17

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Get the memory utilization
float LinuxParser::MemoryUtilization() {
    float mem_total = 0;
    float mem_free = 0;
    float buffers = 0;
    string str;
    std::ifstream stream(LinuxParser::kProcDirectory + LinuxParser::kMeminfoFilename);
    if (stream.is_open()) {
        while (stream >> str) {
            if (str == "MemTotal:") {
                if (stream >> str) {
                    mem_total = stof(str);
                }
            } else if (str == "MemFree:") {
                if (stream >> str) {
                    mem_free = stof(str);
                }
            } else if (str == "Buffers:") {
                if (stream >> str) {
                    buffers = stof(str);
                }
            }
        }
    }

    return 1.0 - mem_free / (mem_total - buffers);
}

// Read and return the system uptime
long LinuxParser::UpTime() {
    std::ifstream stream(LinuxParser::kProcDirectory + LinuxParser::kUptimeFilename);
    if (stream.is_open()) {
        string line;
        string str;
        std::getline(stream, line);
        std::istringstream streamline(line);
        if (streamline >> str) {
            return stol(str);
        }
    }
    return 0;
}

// Calculate the total jiffies from system start to the present time
long LinuxParser::Jiffies() {
    // Total jiffies = uptime seconds of system + number of jiffies per second
    return UpTime() * sysconf(_SC_CLK_TCK);
}

long LinuxParser::ActiveJiffies(int pid) {
    long total_jiffies = 0;
    string line;
    std::ifstream stream(LinuxParser::kProcDirectory + std::to_string(pid) + LinuxParser::kStatFilename);

    if (stream.is_open()) {
        std::getline(stream, line);
        std::istringstream linestream(line);
        vector<string> tokens((std::istream_iterator<string>(linestream)), std::istream_iterator<string>());
        
        for (size_t i = UTIME_INDEX - 1; i <= CSTIME_INDEX - 1; ++i) {
            total_jiffies += stol(tokens[i]);
        }
    }

    return total_jiffies;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
    vector<string> time = CpuUtilization();
    long total_jiffies = stol(time[CPUStates::kUser_]) + stol(time[CPUStates::kNice_]) +
                         stol(time[CPUStates::kSystem_]) + stol(time[CPUStates::kIRQ_]) +
                         stol(time[CPUStates::kSoftIRQ_]) + stol(time[CPUStates::kSteal_]) +
                         stol(time[CPUStates::kGuest_]) + stol(time[CPUStates::kGuestNice_]);

    return total_jiffies;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
    vector<string> time = CpuUtilization();

    return stol(time[CPUStates::kIdle_]) + stol(time[CPUStates::kIOwait_]);
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
    vector<string> cpu;
    string line;
    string str;
    std::ifstream stream(LinuxParser::kProcDirectory + LinuxParser::kStatFilename);

    if (stream.is_open()) {
        while (std::getline(stream, line)) {
            std::istringstream linestream(line);

            while(linestream >> str) {
                if (str == "cpu") {
                    while (linestream >> str) {
                        cpu.push_back(str);
                    }
                    return cpu;
                }
            }
        }
    }
    return cpu;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
    std::ifstream stream(LinuxParser::kProcDirectory + LinuxParser::kStatFilename);
    string line;
    string key;
    int value;

    if (stream.is_open()) {
        while (std::getline(stream, line)) {
            std::istringstream linestream(line);
            while (linestream >> key >> value) {
                if (key == "processes") {
                    return value;
                }
            }
        }
    }

    return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
    std::ifstream stream(LinuxParser::kProcDirectory + LinuxParser::kStatFilename);
    string line;
    string key;
    int value;

    if (stream.is_open()) {
        while (std::getline(stream, line)) {
            std::istringstream linestream(line);

            while (linestream >> key >> value) {
                if (key == "procs_running") {
                    return value;
                }
            }
        }
    }
    return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
    std::ifstream stream(LinuxParser::kProcDirectory + to_string(pid) + LinuxParser::kCmdlineFilename);
    string line;

    if (stream.is_open()) {
        std::getline(stream, line);
    }

    return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
    std::ifstream stream(LinuxParser::kProcDirectory + std::to_string(pid) + LinuxParser::kStatusFilename);
    string line;
    string ram;

    if (stream.is_open()) {
        while (std::getline(stream, line)) {
            std::istringstream linestream(line);
            string key;

            if (linestream >> key >> ram) {
                if (key == "VmSize:") {
                    ram = to_string(stoi(ram) / 1024); // Convert kb to mb
                    return ram;
                }
            }
        }
    }

    return ram;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
    string uid;
    std::ifstream stream(LinuxParser::kProcDirectory + to_string(pid) + LinuxParser::kStatusFilename);
    string line;

    if (stream.is_open()) {
        while (std::getline(stream, line)) {
            std::istringstream linestream(line);
            string key;

            if (linestream >> key) {
                if (key == "Uid:") {
                    if (linestream >> uid)
                    {
                        return uid;
                    }
                }
            }
        }
    }

    return uid;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
    std::ifstream stream(LinuxParser::kPasswordPath);
    string line;
    string str = "x:" + Uid(pid);
    string user;

    if (stream.is_open()) {
        while (std::getline(stream, line)) {
            auto  pos_found = line.find(str);
            if (pos_found != string::npos) {
                user =  line.substr(0, pos_found - 1);
                return user;
            }
        }
    }

    return user; 
}

// Read and return the uptime of a process
long int LinuxParser::UpTime(int pid) {
    long int time = 0;
    string str;
    std::ifstream stream(LinuxParser::kProcDirectory + to_string(pid) + LinuxParser::kStatFilename);

    if (stream.is_open()) {
        for (int i = 1; stream >> str; i++) {
            if (i == 13) {
                time = stol(str) / sysconf(_SC_CLK_TCK);
                return time;
            }
        }
    }

    return time;
}
