#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <cstdint>

#define MAX_CORES 32
#define PORT 1234

#pragma pack(push, 1)
struct CpuPacket {
    double total_load;
    double load_percent[MAX_CORES];
    int32_t core_count;
};
#pragma pack(pop)

class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);
    
signals:
    void errorOccurred(const QString &message);
    void connectionLost();
    void dataPacketReceived(const CpuPacket &packet);

private slots:
    void onReadyRead();
    void onTimeout();

private:
    static constexpr int TIMEOUT_MS = 3000;
    QUdpSocket *m_udpSocket;
    QTimer *m_lostConnectionTimer;
};
#endif