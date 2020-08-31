#include <iostream>

#include "linux_parser.h"
#include "processor.h"

using std::stol;

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  /* user    nice   system  idle      iowait irq   softirq  steal  guest
   * guest_nice */

  auto cpu_str_data = LinuxParser::CpuUtilization();

  auto user = stol(cpu_str_data[LinuxParser::CPUStates::kUser_].c_str());
  auto nice = stol(cpu_str_data[LinuxParser::CPUStates::kNice_].c_str());
  auto system = stol(cpu_str_data[LinuxParser::CPUStates::kSystem_].c_str());
  auto idle = stol(cpu_str_data[LinuxParser::CPUStates::kIdle_].c_str());
  auto iowait = stol(cpu_str_data[LinuxParser::CPUStates::kIOwait_].c_str());

  auto irq = stol(cpu_str_data[LinuxParser::CPUStates::kIRQ_].c_str());
  auto softirq = stol(cpu_str_data[LinuxParser::CPUStates::kSoftIRQ_].c_str());
  auto steal = stol(cpu_str_data[LinuxParser::CPUStates::kSteal_].c_str());
  [[maybe_unused]] auto guest =
      stol(cpu_str_data[LinuxParser::CPUStates::kGuest_].c_str());
  [[maybe_unused]] auto guest_nice =
      stol(cpu_str_data[LinuxParser::CPUStates::kGuestNice_].c_str());

  auto idle_g = idle + iowait;
  auto nonIdle = user + nice + system + irq + softirq + steal;

  auto total = idle_g + nonIdle;

  auto totald = total - Processor::prevTotal;
  auto idled = idle - Processor::prevIdle;

  Processor::prevIdle = idle;
  Processor::prevTotal = total;

  return (totald - idled * 1.0) / totald;
}