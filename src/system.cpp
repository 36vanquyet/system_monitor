#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <algorithm>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

// Return the system's CPU
Processor& System::Cpu() {
    return cpu;
}

// Return a container composed of the system's processes
vector<Process>& System::Processes() {
    // Get the list of process id
    vector<int> pids = LinuxParser::Pids();
    /* Create a set to store existing process id */
    set<int> pids_exist;

    for (Process const& process : this->processes) {
        pids_exist.insert(process.Pid());
    }

    // Emplace the new processes into the vector
    for (int pid : pids) {
        if (pids_exist.find(pid) == pids_exist.end())
        {
            this->processes.emplace_back(pid);
        }
    }

    // Todo Update the CPU Utilization for each process 
    for (auto& process : this->processes)
    {
        process.CpuUtilization(LinuxParser::ActiveJiffies(process.Pid()), LinuxParser::Jiffies());
    }

    // Sort processes by CPU Utilization
    std::sort(this->processes.begin(), this->processes.end(), std::greater<Process>());

    return this->processes;
}

// Return the system's kernel identifier (string)
std::string System::Kernel() const { 
    return LinuxParser::Kernel();
}

// Return the system's memory utilization
float System::MemoryUtilization() const {
    return LinuxParser::MemoryUtilization();
}

// Return the operating system name
std::string System::OperatingSystem() const {
    return LinuxParser::OperatingSystem();
}

// Return the number of processes actively running on the system
int System::RunningProcesses() const {
    return LinuxParser::RunningProcesses();
}

// Return the total number of processes on the system
int System::TotalProcesses() const {
    return LinuxParser::TotalProcesses();
}

// Return the number of seconds since the system started running
long int System::UpTime() const {
    return LinuxParser::UpTime();
}
