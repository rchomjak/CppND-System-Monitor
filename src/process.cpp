#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid, float cpu_util)
    : pid_(pid), cpu_util_(Process::CpuUtilization()){};

// TODO: Return this process's ID
int Process::Pid() { return Process::pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() {
  auto system_uptime = LinuxParser::UpTime();
  auto process_uptime = LinuxParser::UpTime(Process::Pid());
  auto time = system_uptime - process_uptime;

  auto active_jiffies = LinuxParser::ActiveJiffies(Process::Pid());

  return 1.0 * (active_jiffies / sysconf(_SC_CLK_TCK)) / time;
}

// TODO: Return the command that generated this process
string Process::Command() { return LinuxParser::Command(Process::Pid()); }

// TODO: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(Process::Pid()); }

// TODO: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(Process::Pid()); }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(Process::Pid()); }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a) const {
  return Process::cpu_util_ > a.cpu_util_;
}