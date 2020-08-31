#include <iostream>

#include "linux_parser.h"
#include "processor.h"

using std::stol;

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  auto cpu_str_data = LinuxParser::CpuUtilization();

  std::vector<LinuxParser::CPUStates> needs_parse{
      LinuxParser::CPUStates::kUser_,    LinuxParser::CPUStates::kNice_,
      LinuxParser::CPUStates::kSystem_,  LinuxParser::CPUStates::kIdle_,
      LinuxParser::CPUStates::kIOwait_,  LinuxParser::CPUStates::kIRQ_,
      LinuxParser::CPUStates::kSoftIRQ_, LinuxParser::CPUStates::kSteal_,
      LinuxParser::CPUStates::kGuest_,   LinuxParser::CPUStates::kGuestNice_};

  std::vector<long> cpu_util_vals;

  for (auto cpu_state : needs_parse) {
    if (cpu_str_data[cpu_state].empty()) {
      cpu_util_vals.emplace_back(0);
    } else {
      cpu_util_vals.emplace_back(stol(cpu_str_data[cpu_state].c_str()));
    }
  }

  auto idle_g = cpu_util_vals[LinuxParser::CPUStates::kIdle_] +
                cpu_util_vals[LinuxParser::CPUStates::kIOwait_];

  auto non_idle = cpu_util_vals[LinuxParser::CPUStates::kUser_] +
                  cpu_util_vals[LinuxParser::CPUStates::kNice_] +
                  cpu_util_vals[LinuxParser::CPUStates::kSystem_] +
                  cpu_util_vals[LinuxParser::CPUStates::kIRQ_] +
                  cpu_util_vals[LinuxParser::CPUStates::kSoftIRQ_] +
                  cpu_util_vals[LinuxParser::CPUStates::kSteal_];

  auto total = idle_g + non_idle;

  auto totald = total - Processor::prev_total;
  auto idled =
      cpu_util_vals[LinuxParser::CPUStates::kIdle_] - Processor::prev_idle;

  Processor::prev_idle = cpu_util_vals[LinuxParser::CPUStates::kIdle_];
  Processor::prev_total = total;

  return (totald - idled * 1.0) / totald;
}