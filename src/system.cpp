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

Processor& System::Cpu() { return cpu_; }

vector<Process>& System::Processes() {
  processes_.clear();
  for (auto pid : LinuxParser::Pids()) {
    processes_.emplace_back(pid);
  }
  std::sort(processes_.begin(), processes_.end());
  return processes_;
}
std::string System::Kernel() {
  if (System::kernel_.empty()) {
    System::kernel_ = LinuxParser::Kernel();
  }

  return System::kernel_;
}

float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

std::string System::OperatingSystem() {
  if (System::os_.empty()) {
    System::os_ = LinuxParser::OperatingSystem();
  }

  return System::os_;
}

int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

long int System::UpTime() { return LinuxParser::UpTime(); }
