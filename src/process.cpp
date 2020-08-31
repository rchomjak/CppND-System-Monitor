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

Process::Process(int pid) : pid_(pid), cpu_util_(Process::CpuUtilization()){};

// TODO: Return this process's ID
int Process::Pid() { return Process::pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() {
  // I read the chat(hub.udacity) to get the information and stackoverflow to
  // calculate cpuutilization
  auto system_uptime = LinuxParser::UpTime();
  auto process_uptime = LinuxParser::UpTime(Process::Pid());
  auto time_diff = system_uptime - process_uptime;

  auto active_jiffies = LinuxParser::ActiveJiffies(Process::Pid());

  return 1.0 * (active_jiffies / sysconf(_SC_CLK_TCK)) / time_diff;
}

// TODO: Return the command that generated this process
string Process::Command() {
  if (Process::command_line.empty()) {
    Process::command_line = LinuxParser::Command(Process::Pid());
  }

  return Process::command_line;
}

// TODO: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(Process::Pid()); }

// TODO: Return the user (name) that generated this process
string Process::User() {
  if (Process::user_name.empty()) {
    Process::user_name = LinuxParser::User(Process::Pid());
  }

  return Process::user_name;
}
// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(Process::Pid()); }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a) const {
  return Process::cpu_util_ > a.cpu_util_;
}