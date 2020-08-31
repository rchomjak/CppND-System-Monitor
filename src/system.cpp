#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"
#include "system.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// TODO: Return the system's CPU
Processor& System::Cpu() { return cpu_; }

// TODO: Return a container composed of the system's processes
vector<Process>& System::Processes() {
  processes_.clear();
  for (auto pid : LinuxParser::Pids()) {
    processes_.emplace_back(Process(pid, 0.0));
  }
  std::sort(processes_.begin(), processes_.end());
  return processes_;
}
// TODO: Return the system's kernel identifier (string)
std::string System::Kernel() {
  if (System::kernel_.empty()) {
    System::kernel_ = LinuxParser::Kernel();
  }

  return System::kernel_;
}

// TODO: Return the system's memory utilization
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

// TODO: Return the operating system name
std::string System::OperatingSystem() {
  if (System::os_.empty()) {
    System::os_ = LinuxParser::OperatingSystem();
  }

  return System::os_;
}

// TODO: Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// TODO: Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// TODO: Return the number of seconds since the system started running
long int System::UpTime() { return LinuxParser::UpTime(); }
