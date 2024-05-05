#include "processor.h"
#include "linux_parser.h"

// Return the aggregate CPU utilization
float Processor::Utilization() {
    float utilization = 0;
    long current_active = LinuxParser::ActiveJiffies();
    long current_idle = LinuxParser::IdleJiffies();
    long duration_active = current_active - previous_active;
    long duration_idle = current_idle - previous_idle;
    long total_duration = duration_active + duration_idle;

    utilization = static_cast<float>(duration_active) / total_duration;
    previous_active = current_active;
    previous_idle = current_idle;

    return utilization;
}
