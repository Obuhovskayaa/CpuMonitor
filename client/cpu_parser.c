#include <stdio.h>
#include <string.h>
#include "cpu_parser.h"

int read_cpu_data(struct CpuTicks *cores) {
    FILE *file = fopen("/proc/stat", "r");
    if (!file) {
        perror("File open failed '/proc/stat'.\n");
        return 0;
    }

    char buffer[256];
    int count = 0;

    while (fgets(buffer, sizeof(buffer), file) && count < MAX_CORES + 1) {
        if (strncmp(buffer, "cpu", 3) == 0) {
            int scanned = sscanf(buffer,
                "%s %llu %llu %llu %llu %llu %llu %llu %llu",
                cores[count].name,
                &cores[count].user,
                &cores[count].nice,
                &cores[count].system,
                &cores[count].idle,
                &cores[count].iowait,
                &cores[count].irq,
                &cores[count].softirq,
                &cores[count].steal
            );
            if (scanned >= 5) {
                count++;
            }
        } else {
            continue;
        }
    }   
    fclose(file);
    return count;
}