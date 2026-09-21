#ifndef CPU_TABS_WIDGET_H
#define CPU_TABS_WIDGET_H

#include <QWidget>
#include <QVector>
#include "NetworkManager.h"

class QTabWidget;
class QCustomPlot;
class QCPRange;

class CpuTabsWidget : public QWidget {
    Q_OBJECT
public:
    explicit CpuTabsWidget(QWidget *parent = nullptr);
    
    void initTabs(const CpuPacket &packet);
    void updateTabs(const CpuPacket &packet);
    void onConnectionLost();

signals:
    void connectionRestored();
//private slots:
//    void handleRangeChanged(const QCPRange &newRange);

private:
    void setupPlotStyle(QCustomPlot *plot, int index);
    QColor getGraphColor(int index);
    double m_startTime;
    QTabWidget *m_tabWidget;
    QCustomPlot *m_totalCpuPlot;
    QVector<QCustomPlot*> m_perCpuPlots;
    double m_maxAllowedX = 0.0;

    bool m_wasDisconnected = false;
    static constexpr double m_windowSize = 300.0;
    static constexpr double m_extendStep = 300.0;
    static constexpr int    m_tickCount  = 6;
};

#endif