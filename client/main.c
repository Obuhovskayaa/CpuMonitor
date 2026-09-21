#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>

#include "cpu_parser.h"

#define PORT 1234
//#define IP_ADDR "127.0.0.1"
//#define IP_ADDR "192.168.0.153"

#pragma pack(push, 1)
struct CpuPacket {
    double total_load;
    double load_percent[MAX_CORES];
    int32_t core_count;
};
#pragma pack(pop) 

int main() {
    struct CpuTicks prev[MAX_CORES + 1];
    struct CpuTicks curr[MAX_CORES + 1];
    struct CpuPacket packet;

    memset(&packet, 0, sizeof(packet));

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed.\n");
        return 1;
    }

    struct sockaddr_in windows_addr;
    memset(&windows_addr, 0, sizeof(windows_addr));
    windows_addr.sin_family = AF_INET;
    windows_addr.sin_port = htons(PORT);
    windows_addr.sin_addr.s_addr = inet_addr(IP_ADDR);

    int entry_count = read_cpu_data(prev);
    if (entry_count == 0) {
        printf("Data read failed.\n");
        close(sock);
        return 1;
    }

    if (entry_count > MAX_CORES + 1) {
        entry_count = MAX_CORES + 1;
    }

    packet.core_count = entry_count - 1;

    #ifdef DEBUG
    printf("UDP-Client started.\n");
    #endif

    while (1) {
        sleep(1);
        read_cpu_data(curr);

        for (int i = 0; i < entry_count; ++i) {
            ull_t prev_work = prev[i].user + prev[i].nice + prev[i].system +
                            prev[i].irq + prev[i].softirq + prev[i].steal;
            ull_t prev_idle = prev[i].idle + prev[i].iowait;
            ull_t prev_total = prev_work + prev_idle;

            ull_t curr_work = curr[i].user + curr[i].nice + curr[i].system +
                            curr[i].irq + curr[i].softirq + curr[i].steal;
            ull_t curr_idle = curr[i].idle + curr[i].iowait;
            ull_t curr_total = curr_work + curr_idle;

            ull_t delta_total = curr_total - prev_total;
            ull_t delta_idle = curr_idle - prev_idle;

            double load_percent = 0.0;
            if (delta_total > 0) {
                load_percent = 100.0 * ((double)(delta_total - delta_idle) / delta_total);
            }
            if (i == 0) {
                packet.total_load = load_percent;
            }
            else {
                packet.load_percent[i - 1] = load_percent;
            }
        }
        #ifdef DEBUG
        printf("Sending packet: Cores=%d, TotalLoad=%.1f%%\n", packet.core_count, packet.total_load);
        #endif
        int bytes_sent = sendto(sock, &packet, sizeof(packet), 0,
                        (struct sockaddr*)&windows_addr, sizeof(windows_addr));
        if (bytes_sent < 0) {
            perror("Send failed");
        }
        memcpy(prev, curr, sizeof(struct CpuTicks) * entry_count);
    }
    close(sock);
    return 0;
}