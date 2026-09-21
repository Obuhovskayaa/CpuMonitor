#ifndef APP_CORE_H
#define APP_CORE_H

#include "NetworkManager.h"
#include <QObject>

class AppCore : public QObject {
    Q_OBJECT
public:
    explicit AppCore(QObject *parent = nullptr);

signals:
    void firstDataReceived(const CpuPacket &packet);
    void dataUpdated(const CpuPacket &packet);
    void connectionLost();

private slots:
    void handleDataPacket(const CpuPacket &packet);

private:
    NetworkManager *m_network;
    bool m_isFirstPacket;

};
#endif