#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "linux_parser.h"

using std::stof;
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
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
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
  string line;
  string key;
  string value;

  float memoryFree;
  float memoryTotal;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::remove_if(line.begin(), line.end(), isspace);
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal") {
          value = value.erase(value.length() - 2); //remove KB from the end
          memoryTotal = std::stof(value);
        }
        if (key == "MemFree") {
          value = value.erase(value.length() - 2); //remove KB from the end
          memoryFree = std::stof(value);
        }
      }
    }
  }
  return (memoryTotal - memoryFree) / memoryTotal; 
}

long LinuxParser::UpTime() { 
  string line;
  string uptimeString;
  string idletimeString;

  float uptime;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> uptimeString >> idletimeString) {
        uptime = std::stol(uptimeString);
      }
    }
  }
  return uptime; 
 }

long LinuxParser::CpuUtilization(int pid) { 
  string line;
  vector<std::string> values;
  size_t pos = 0;

  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    while((pos = line.find(" ")) != std::string::npos) {
      values.push_back(line.substr(0, pos));
      line.erase(0, pos + 1);
    }
  }

  long utime = stol(values[13]);
  long stime = stol(values[14]);
  long cutime = stol(values[15]);
  long cstime = stol(values[16]);
  long uptime = UpTime();
  long starttime = UpTime(pid);
  long hertz = sysconf(_SC_CLK_TCK);

  long totalTime = utime + stime + cutime + cstime;
  long seconds = uptime - starttime;

  if(seconds) {
    return 100*((totalTime / hertz) / seconds);
  }
  return 0;
 }

float LinuxParser::CpuUtilization() { 
  string line;
  string substring;
  vector<std::string> values;
  size_t pos = 0;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    while((pos = line.find(" ")) != std::string::npos) {
      substring = line.substr(0, pos);
      values.push_back(line.substr(0, pos));
      line.erase(0, pos + 1);
    }
  }


  float user = stof(values[2]);
  float nice = stof(values[3]);
  float system = stof(values[4]);
  float idle = stof(values[5]);
  float iowait = stof(values[6]);
  // float irq = stof(values[6]);
  // float softIrq = stof(values[7]);
  // float steal = stof(values[8]);
  // float guest = stof(values[9]);
  // float guestNice = stof(values[10]);

  float idletime = idle + iowait;
  float busytime =  user + nice + system;

  return (busytime / (idletime + busytime));
}

int LinuxParser::TotalProcesses() { 
  string line;
  string key;
  string value;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          return std::stoi(value);
        }
      }
    }
  }
  return 0;
}

int LinuxParser::RunningProcesses() { 
  string line;
  string key;
  string value;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          return std::stoi(value);
        }
      }
    }
  }
  return 0;
 }

string LinuxParser::Command(int pid) { 
  string line;
  string key;
  string value;

  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    return line;
  }
  return "";
 }

string LinuxParser::Ram(int pid) { 
  string line;
  string key;
  string value;

  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::remove_if(line.begin(), line.end(), isspace);
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize") {
          value = value.erase(value.length() - 2); //remove KB from the end
          return std::to_string(stoi(value)/1000) + "MB"; //convert to MB
        }
      }
    }
  }
  return ""; 
}

string LinuxParser::Uid(int pid) { 
  string line;
  string key;
  string value;

  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
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
  return "";
 }

string LinuxParser::User(int pid) {
  string line;
  string key;
  string x;
  string id;

  string userId = Uid(pid);

  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::remove_if(line.begin(), line.end(), isspace);
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> x >> id) {
        if (id == userId) {
          return key;
        }
      }
    }
  }

  return ""; 
 }

long LinuxParser::UpTime(int pid) { 
  string line;
  string key;
  string value;

  vector<std::string> values;
  size_t pos = 0;

  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    while((pos = line.find(" ")) != std::string::npos) {
      values.push_back(line.substr(0, pos));
      line.erase(0, pos + 1);
    }
  }
  return stol(values[21])/sysconf(_SC_CLK_TCK);
 }
