#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
public:
    Process(int pid);
    int Pid() const;                              
    std::string User() const;                     
    std::string Command() const;                  
    float CpuUtilization() const;                  
    void CpuUtilization(long active_jiffies, long jiffies);
    std::string Ram() const;                      
    long int UpTime() const;                      
    bool operator>(Process const& a) const;  

private:
    int pid;
    float cpu_utilization = 0;
    long active_jiffies = 0;
    long jiffies = 0;
};

#endif
