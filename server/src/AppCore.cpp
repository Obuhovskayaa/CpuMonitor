#include "AppCore.h"

AppCore::AppCore(QObject *parent) :
    QObject(parent), m_network(new NetworkManager(this)),
    m_isFirstPacket(true)
{
    connect(m_network, &NetworkManager::dataPacketReceived, this, &AppCore::handleDataPacket);
    connect(m_network, &NetworkManager::connectionLost, [this](){
        emit connectionLost();
    });
}

void AppCore::handleDataPacket(const CpuPacket &packet) {
    if (m_isFirstPacket) {
        m_isFirstPacket = false;
        emit firstDataReceived(packet);
    } else {
        emit dataUpdated(packet);
    }
}
    