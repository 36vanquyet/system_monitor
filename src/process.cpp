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

Process::Process(int pid) {
    this->pid = pid;
};

// Return this process's ID
int Process::Pid() const {
    return this->pid;
}

// Return this process's CPU utilization
float Process::CpuUtilization() const {
    return cpu_utilization;
}

void Process::CpuUtilization(long active_jiffies, long jiffies)
{
    long duration_jiffies = jiffies - this->jiffies;
    long duration_active_jiffies = active_jiffies - this->active_jiffies;

    this->cpu_utilization = static_cast<float>(duration_active_jiffies) / duration_jiffies;

    this->jiffies = jiffies;
    this->active_jiffies = active_jiffies;
}
// Return the command that generated this process
string Process::Command() const {
    return LinuxParser::Command(this->pid);
}

// Return this process's memory utilization
string Process::Ram() const {
    return LinuxParser::Ram(this->pid);
}

// Return the user (name) that generated this process
string Process::User() const {
    return LinuxParser::User(this->pid);
}

// Return the age of this process (in seconds)
long int Process::UpTime() const {
    return LinuxParser::UpTime(this->pid);
}

// Overload the "greater than" comparison operator for Process objects
bool Process::operator>(Process const& a) const {
    return CpuUtilization() > a.CpuUtilization();
}
