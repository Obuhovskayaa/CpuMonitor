#include "NetworkManager.h"
#include <QMessageBox>

NetworkManager::NetworkManager(QObject *parent) 
    : QObject(parent),
        m_lostConnectionTimer(new QTimer(this)), 
        m_udpSocket(new QUdpSocket(this))
{
    m_lostConnectionTimer->setInterval(TIMEOUT_MS);
    m_lostConnectionTimer->setSingleShot(true);
    connect(m_lostConnectionTimer, &QTimer::timeout, this, &NetworkManager::onTimeout);

    if (!m_udpSocket->bind(QHostAddress::Any, PORT)) {
        emit errorOccurred("cant get this port.\n");
    }
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &NetworkManager::onReadyRead);
}

void NetworkManager::onReadyRead() {
    while (m_udpSocket->hasPendingDatagrams()) {
        m_lostConnectionTimer->start();
        #ifdef DEBUG
        qDebug() << "=== UDP pack size(bytes):" << m_udpSocket->pendingDatagramSize();
        qDebug() << "=== expected size of CpuPack:" << sizeof(CpuPacket);
        #endif
        CpuPacket packet;
        m_udpSocket->readDatagram(reinterpret_cast<char*>(&packet), sizeof(packet));
        #ifdef DEBUG
        qDebug() << "=== Cpu count:" << packet.core_count 
                 << "total load:" << packet.total_load;
        #endif
        emit dataPacketReceived(packet);
    }
}

void NetworkManager::onTimeout() {
    emit connectionLost();
}