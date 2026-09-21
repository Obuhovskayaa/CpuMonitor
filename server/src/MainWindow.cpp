#include "MainWindow.h"
#include "AppCore.h"
#include "CpuTabsWidget.h"
#include <QStackedWidget>
#include <QLabel>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) 
    : QMainWindow(parent), m_stackedWidget(new QStackedWidget(this)),
        m_tabsWidget(new CpuTabsWidget(this)) 
{
    resize(800, 500);
    setCentralWidget(m_stackedWidget);

    QWidget *loadingPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(loadingPage);
    QLabel *label = new QLabel("Waiting for Linux data...");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    m_stackedWidget->addWidget(loadingPage);
    m_stackedWidget->addWidget(m_tabsWidget);

    m_stackedWidget->setCurrentIndex(0);
}

void MainWindow::initConnections(AppCore *core) {
    connect(core, &AppCore::firstDataReceived, this, [this](const CpuPacket &packet){
        m_tabsWidget->initTabs(packet);
        m_stackedWidget->setCurrentIndex(1);
        setWindowTitle("Status: connected.");
    });
    
    connect(core, &AppCore::dataUpdated, m_tabsWidget, &CpuTabsWidget::updateTabs);
    connect(core, &AppCore::connectionLost, this, [this](){
        setWindowTitle("Status: connection lost...");
        m_tabsWidget->onConnectionLost();
    });

    connect(m_tabsWidget, &CpuTabsWidget::connectionRestored, this, [this](){
        setWindowTitle("Status: connected.");
    });
}