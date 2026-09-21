#ifndef CPU_PARSER_H
#define CPU_PARSER_H

#define MAX_CORES 32

typedef unsigned long long ull_t;

struct CpuTicks {
    char name[10];
    ull_t user, nice, system, idle, iowait, irq, softirq, steal;
};

int read_cpu_data(struct CpuTicks *cores);

#endif