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
    filestream.close();
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;

    stream.close();
  }
  return kernel;
}

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

float LinuxParser::MemoryUtilization() {
  auto mem_total = LinuxParser::findValueByKey<float>(
      "MemTotal:", kProcDirectory + kMeminfoFilename);
  auto mem_free = LinuxParser::findValueByKey<float>(
      "MemFree:", kProcDirectory + kMeminfoFilename);
  auto buffers = LinuxParser::findValueByKey<float>(
      "Buffers:", kProcDirectory + kMeminfoFilename);
  auto cached = LinuxParser::findValueByKey<float>(
      "Cached:", kProcDirectory + kMeminfoFilename);

  auto total_used_memory = mem_total - mem_free;
  auto non_cached_memory = total_used_memory - (buffers + cached);

  return non_cached_memory / mem_total;
}

long LinuxParser::UpTime() {
  string line;

  string uptime;
  string idle_uptime;

  long uptime_l;

  std::ifstream filestream(kProcDirectory + kUptimeFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line) &&
           (uptime.empty() || idle_uptime.empty())) {
      std::istringstream linestream(line);
      linestream >> uptime >> idle_uptime;
    }
    filestream.close();
  }

  uptime_l = stol(uptime.c_str());

  return uptime_l;
}

long LinuxParser::Jiffies() {
  return LinuxParser::IdleJiffies() + LinuxParser::ActiveJiffies();
}

long LinuxParser::ActiveJiffies(int pid) {
  auto proc_cpu_util_vec = LinuxParser::ProcCpuUtilization(pid);

  vector<ProcStats> needs_parse{kUtime_, kStime_};
  vector<long> active_jiffies_vals;

  for (auto cpu_state : needs_parse) {
    if (proc_cpu_util_vec[cpu_state].empty()) {
      active_jiffies_vals.emplace_back(0);
    } else {
      active_jiffies_vals.emplace_back(
          stol(proc_cpu_util_vec[cpu_state].c_str()));
    }
  }

  long active_jiffies_cpu{0};

  for (auto util_val : active_jiffies_vals) {
    active_jiffies_cpu += util_val;
  }

  return active_jiffies_cpu;
}

long LinuxParser::ActiveJiffies() {
  auto cpu_utilization = LinuxParser::CpuUtilization();

  vector<CPUStates> needs_parse{kUser_,  kNice_,      kSystem_,
                                kGuest_, kGuestNice_, kSteal_};
  vector<long> cpu_util_vals;

  for (auto cpu_state : needs_parse) {
    if (cpu_utilization[cpu_state].empty()) {
      cpu_util_vals.emplace_back(0);
    } else {
      cpu_util_vals.emplace_back(stol(cpu_utilization[cpu_state].c_str()));
    }
  }

  long util_cpu{0};

  for (auto util_val : cpu_util_vals) {
    util_cpu += util_val;
  }

  return util_cpu;
}

long LinuxParser::IdleJiffies() {
  auto cpu_utilization = LinuxParser::CpuUtilization();

  vector<CPUStates> needs_parse{kIdle_, kIRQ_, kSoftIRQ_};

  vector<long> idle_jiffies_vals;

  for (auto cpu_state : needs_parse) {
    if (cpu_utilization[cpu_state].empty()) {
      idle_jiffies_vals.emplace_back(0);
    } else {
      idle_jiffies_vals.emplace_back(stol(cpu_utilization[cpu_state].c_str()));
    }
  }

  long idle_jiffies_cpu{0};

  for (auto util_val : idle_jiffies_vals) {
    idle_jiffies_cpu += util_val;
  }

  return idle_jiffies_cpu;
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
        cpu_proc_data.emplace_back(value);
      }
    }

    filestream.close();
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
            cpu_data.emplace_back(value);
          }
        }
      }
    }
    filestream.close();
  }

  return cpu_data;
}

int LinuxParser::TotalProcesses() {
  return LinuxParser::findValueByKey<int>("processes",
                                          kProcDirectory + kStatFilename);
}

int LinuxParser::RunningProcesses() {
  return LinuxParser::findValueByKey<int>("procs_running",
                                          kProcDirectory + kStatFilename);
}

string LinuxParser::Command(int pid) {
  string line;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);

  if (filestream.is_open()) {
    getline(filestream, line);
    filestream.close();
  }

  return line;
}

string LinuxParser::Ram(int pid) {
  auto vm_data = LinuxParser::findValueByKey<std::string>(
      "VmData:", kProcDirectory + to_string(pid) + kStatusFilename);

  return vm_data.empty() ? nullptr : to_string(stol(vm_data) / 1024);
}

string LinuxParser::Uid(int pid) {
  auto uid = LinuxParser::findValueByKey<std::string>(
      "Uid:", kProcDirectory + to_string(pid) + kStatusFilename);

  return uid;
}

string LinuxParser::User(int pid) {
  auto uid = LinuxParser::Uid(pid);

  if (uid.empty()) {
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
    filestream.close();
  }

  return string("ERR-UNKNOWN_USER");
}

long LinuxParser::UpTime(int pid) {
  string line;
  string value;
  string uptime{"0"};

  size_t uptime_cnt{0};

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (uptime_cnt < 22 && linestream >> value) {
        uptime = value;
        uptime_cnt += 1;
      }
    }
    filestream.close();
  }

  auto uptime_clock_ticks = stol(uptime);
  uptime_clock_ticks =
      LinuxParser::UpTime() - (uptime_clock_ticks / sysconf(_SC_CLK_TCK));

  return uptime_clock_ticks;
}