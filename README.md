# Linux CPU Monitor

A system for collecting and visualizing data on CPU core load.

## 1. Client / Parser

This part is a lightweight Linux service written in **pure C**.
It combines a low-overhead system data parser
and a network client that streams parsed data to a remote server.

### How it works:

It reads raw CPU load data from the Linux ***/proc/stat*** system file at a 1 Hz frequency.
It calculates the active cores and the load on each of them, and records the results in a data packet
(Currently, the number of cores is strictly limited by a constant to avoid dynamic memory allocation).
It then transmits them via an IPv4 UDP socket directly to the destination port on the host.

### Build and run

**./main.c:**   
If running both server and client on the same Linux machine (localhost), left #define IP_ADDR "127.0.0.1". Otherwise, set it to your host machine's IP

```bash
gcc main.c cpu_parser.c -o cpu_client
./cpu_client
```
## 2. Cross-Platform GUI Server (Qt 6)

This component is a desktop application with a graphical interface, written in **C++/Qt 6**. It is used to display data from the client in real time.


### How it works

The graphical interface is entirely written by hand, without using Qt Designer.

**QUdpSocket** is used. The socket automatically generates the **readyRead** signal when a new data packet arrives. The application intercepts this signal and immediately processes the packet asynchronously.

The **QCustomPlot** library is used to create graphs.
The application is waiting to receive the first data packet in order to determine the exact number of cores. After receiving the data, it creates separate user interface tabs with graphs for each processor core, as well as a tab with general information about the system load.

### Build and run

You can just open the project file **CMakeLists.txt** directly in **Qt Creator**, select Qt 6 Kit, and hit the **Run** button.

To build via terminal using CMake:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```




