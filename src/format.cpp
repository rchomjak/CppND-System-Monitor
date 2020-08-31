#include <string>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long in_seconds) {
  long hours{0};
  long minutes{0};
  long seconds{0};

  seconds = in_seconds;
  if (in_seconds >= 60) {
    minutes = seconds / 60;
    seconds = seconds - minutes * 60;
  }

  if (minutes >= 60) {
    hours = minutes / 60;
    minutes = minutes - hours * 60;
  }

  return std::to_string(hours) + ":" + std::to_string(minutes) + ":" +
         std::to_string(seconds);
}