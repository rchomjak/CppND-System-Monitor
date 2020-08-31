#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::stol;
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

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  string value;
  string memTotal;
  string memFree;
  string memAvailable;
  string buffers;
  string cached;

  float memTotal_f;
  float memFree_f;
  float memAvailable_f;
  float buffers_f;
  float cached_f;

  float totalUsedMemory_f;
  float nonCachedMemory_f;

  std::ifstream filestream(kProcDirectory + kMeminfoFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line) &&
           (memTotal.empty() || memFree.empty() || memAvailable.empty() ||
            buffers.empty() || cached.empty())) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          memTotal = value;
        } else if (key == "MemFree:") {
          memFree = value;
        } else if (key == "MemAvailable:") {
          memAvailable = value;
        } else if (key == "Buffers:") {
          buffers = value;
        } else if (key == "Cached:") {
          cached = value;
        }
      }
    }
  }

  memTotal_f = stof(memTotal.c_str());
  memFree_f = stof(memFree.c_str());
  memAvailable_f = stof(memAvailable.c_str());
  buffers_f = stof(buffers.c_str());
  cached_f = stof(cached.c_str());

  totalUsedMemory_f = memTotal_f - memFree_f;
  nonCachedMemory_f = totalUsedMemory_f - (buffers_f + cached_f);

  return nonCachedMemory_f / memTotal_f;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  string line;

  string uptime;
  string idle_uptime;

  long uptime_l;
  long idle_uptime_l;

  std::ifstream filestream(kProcDirectory + kUptimeFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line) &&
           (uptime.empty() || idle_uptime.empty())) {
      std::istringstream linestream(line);
      linestream >> uptime >> idle_uptime;
    }
  }

  uptime_l = stol(uptime.c_str());
  idle_uptime_l = stol(idle_uptime.c_str());

  return uptime_l;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::IdleJiffies() + LinuxParser::ActiveJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  auto proc_cpu_util_vec = LinuxParser::ProcCpuUtilization(pid);

  auto utime = stol(proc_cpu_util_vec[LinuxParser::ProcStats::kUtime_]);
  auto stime = stol(proc_cpu_util_vec[LinuxParser::ProcStats::kStime_]);

  return utime + stime;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto cpu_utilization = LinuxParser::CpuUtilization();

  auto user = stol(cpu_utilization[LinuxParser::CPUStates::kUser_].c_str());
  auto nice = stol(cpu_utilization[LinuxParser::CPUStates::kNice_].c_str());

  auto system = stol(cpu_utilization[LinuxParser::CPUStates::kSystem_].c_str());

  auto guest = stol(cpu_utilization[LinuxParser::CPUStates::kGuest_].c_str());
  auto guest_nice =
      stol(cpu_utilization[LinuxParser::CPUStates::kGuestNice_].c_str());

  auto steal = stol(cpu_utilization[LinuxParser::CPUStates::kSteal_].c_str());

  return user + nice + system + guest + guest_nice + steal;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  auto cpu_utilization = LinuxParser::CpuUtilization();

  auto idle = stol(cpu_utilization[LinuxParser::CPUStates::kIdle_].c_str());
  auto irq = stol(cpu_utilization[LinuxParser::CPUStates::kIRQ_].c_str());
  auto softirq =
      stol(cpu_utilization[LinuxParser::CPUStates::kSoftIRQ_].c_str());

  return idle + irq + softirq;
}

vector<string> LinuxParser::ProcCpuUtilization(int pid) {
  string line;
  string value;

  vector<string> cpu_proc_data;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> value) {
        cpu_proc_data.push_back(value);
      }
    }
  }

  return cpu_proc_data;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line;
  string key;
  string value;
  string cpu;

  vector<string> cpu_data;

  std::ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line) && cpu.empty()) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "cpu") {
          cpu = value;
          while (linestream >> value) {
            // user    nice   system  idle      iowait irq   softirq  steal
            // guest guest_nice 20982172 29206 4608000 36509003    393325    0
            // 401402   0        0       0
            cpu_data.push_back(value);
          }
        }
      }
    }
  }

  return cpu_data;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string key;
  string value;
  string totalProcesses;
  int totalProcesses_i;

  std::ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line) && totalProcesses.empty()) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          totalProcesses = value;
        }
      }
    }
  }

  totalProcesses_i = atoi(totalProcesses.c_str());
  return totalProcesses_i;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  string value;
  string runningProcesses;
  int runningProcesses_i;

  std::ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line) && runningProcesses.empty()) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          runningProcesses = value;
          break;
        }
      }
    }
  }

  runningProcesses_i = atoi(runningProcesses.c_str());
  return runningProcesses_i;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string line;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);

  if (filestream.is_open()) {
    getline(filestream, line);
  }

  return line;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string line;
  string key;
  string value;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          return to_string((stol(value) / 1024));
        }
      }
    }
  }

  return to_string(0);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string line;
  string key;
  string value;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") {
          return value;
        }
      }
    }
  }

  return to_string(-1);
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  auto uid = LinuxParser::Uid(pid);

  if (uid == "-1") {
    return string("ERR-UNKNOWN_USER");
  }

  string line;
  string key;
  string value;

  string username_;
  string uid_;
  string passwd_;

  std::ifstream filestream(kPasswordPath);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);

      linestream >> username_;
      linestream >> passwd_;
      linestream >> uid_;

      if (uid_ == uid) {
        return username_;
      }
    }
  }

  return string("ERR-UNKNOWN_USER");
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  string line;
  string value;
  string uptime;

  size_t uptime_cnt{0};

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line) && uptime.empty()) {
      std::istringstream linestream(line);
      while (uptime_cnt < 22 && linestream >> value) {
        uptime = value;
        uptime_cnt += 1;
      }
    }
  }

  auto uptime_clock_ticks = stol(uptime);
  uptime_clock_ticks /= sysconf(_SC_CLK_TCK);
  return uptime_clock_ticks;
}