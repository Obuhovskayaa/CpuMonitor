#include "CpuTabsWidget.h"
#include "qcustomplot.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QDateTime>
#include <QColor>
#include <QSharedPointer>

static void updateArrowPosition(QCustomPlot *plot) {
    if (!plot->graph(0) || plot->graph(0)->data()->isEmpty()) return;

    int dataCount = plot->graph(0)->dataCount();
    auto dataContainer = plot->graph(0)->data();

    double lastTime = dataContainer->at(dataCount - 1)->key;
    double lastValue = dataContainer->at(dataCount - 1)->value;

    QVariant markerProp = plot->property("arrowMarker");
    if (markerProp.isValid()) {
        QCPItemText *marker = markerProp.value<QCPItemText*>();
        if (marker) {
            double rectRight = plot->axisRect()->right();
            double xAxisUpper = plot->xAxis->range().upper;
            
            bool isBehindScreen = (lastTime > xAxisUpper);

            if (marker->visible() != isBehindScreen) {
                marker->setVisible(isBehindScreen);
            }
            if (isBehindScreen) {
                marker->position->setCoords(rectRight + 2, lastValue);
                marker->setText(QString("⯇ %1%").arg(lastValue, 0, 'f', 1));
            }
        }
    }
}

CpuTabsWidget::CpuTabsWidget(QWidget *parent) 
    : QWidget(parent), m_tabWidget(new QTabWidget(this)),
    m_totalCpuPlot(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_tabWidget);
}

void CpuTabsWidget::initTabs(const CpuPacket &packet) {
    if (m_totalCpuPlot) return;
    
    QDateTime nowTime = QDateTime::currentDateTime();
    QTime cleanMinuteTime(nowTime.time().hour(), nowTime.time().minute(), 0, 0);
    nowTime.setTime(cleanMinuteTime);

    m_startTime = nowTime.toMSecsSinceEpoch() / 1000.0;
    m_maxAllowedX = m_startTime + m_windowSize;

    double realCurrentTime =  QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0;

    auto createSinglePlot = [&](int index) -> QCustomPlot* {
        QWidget *page = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->setContentsMargins(4, 4, 4, 4);

        QCustomPlot *plot = new QCustomPlot(page);
        plot->addGraph();

        setupPlotStyle(plot, index);

        double firstData = (index == -1) ? packet.total_load : packet.load_percent[index];
        plot->graph(0)->addData(realCurrentTime, firstData);
        
        layout->addWidget(plot);
        QString tabTitle = (index == -1) ? "Total" : QString("Cpu %1").arg(index);
        m_tabWidget->addTab(page, tabTitle);

        return plot;
    };

    m_totalCpuPlot = createSinglePlot(-1);

    m_perCpuPlots.reserve(packet.core_count);
    for (int i = 0; i < packet.core_count; ++i) {
        m_perCpuPlots.append(createSinglePlot(i));       
    }
}

// void CpuTabsWidget::handleRangeChanged(const QCPRange &newRange) {
    
// }

void CpuTabsWidget::updateTabs(const CpuPacket &packet) {
    if (!m_totalCpuPlot) return;

    double nowTime = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0;

    if (nowTime > m_maxAllowedX) {
        m_maxAllowedX += m_extendStep;
    }

    auto updatePlotData = [this](QCustomPlot *plot, double time, double data) {
        if (m_wasDisconnected) {
            plot->graph(0)->addData(time - 0.001, qQNaN());
        }

        plot->graph(0)->addData(time, data);
        
        QVariant prop = plot->property("valueLabel");
        if (prop.isValid()) {
            QCPTextElement *label = prop.value<QCPTextElement*>();
            if (label) {
                label->setText(QString("CPU: %1%").arg(data, 0, 'f', 1));
            }
        }

        updateArrowPosition(plot);

        plot->replot(); 
    };

    updatePlotData(m_totalCpuPlot, nowTime, packet.total_load);

    for (int i = 0; i < packet.core_count; ++i) {
        updatePlotData(m_perCpuPlots[i], nowTime, packet.load_percent[i]);
    }

    if (m_wasDisconnected) {
        m_wasDisconnected = false;
        emit connectionRestored();
    }
}

void CpuTabsWidget::setupPlotStyle(QCustomPlot *plot, int index) {
    plot->axisRect()->setAutoMargins(QCP::msLeft | QCP::msTop | QCP::msBottom);
    plot->axisRect()->setMargins(QMargins(0, 0, 75, 0));

    plot->yAxis->setRange(0, 100);
    plot->yAxis->setLabel("");

    QFont font("sans-serif", 9);
    font.setBold(true);

    QCPTextElement *yAxisLabel = new QCPTextElement(plot, "CPU, %", font);
    yAxisLabel->setTextFlags(Qt::AlignLeft | Qt::AlignBottom);
    yAxisLabel->setMargins(QMargins(0, 0, 0, -100));
    plot->plotLayout()->insertRow(0);
    plot->plotLayout()->addElement(0, 0, yAxisLabel);

    auto dateTimeTicker = QSharedPointer<QCPAxisTickerDateTime>::create();

    dateTimeTicker->setDateTimeFormat("hh:mm");
    dateTimeTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
    dateTimeTicker->setTickCount(m_tickCount);

    plot->xAxis->setTicker(dateTimeTicker);
    plot->xAxis->setRange(m_startTime + 0.5, m_startTime + m_windowSize);
    plot->graph(0)->setPen(QPen(getGraphColor(index), 2));

    plot->setInteractions(QCP::iRangeDrag);
    plot->axisRect()->setRangeDrag(Qt::Horizontal);
    
    connect(plot->xAxis, qOverload<const QCPRange&>(&QCPAxis::rangeChanged), this,
        [this, plot](const QCPRange &newRange) {
            QCPRange boundedRange = newRange.bounded(m_startTime + 0.5, m_maxAllowedX);
            if (boundedRange != newRange) {
                plot->xAxis->setRange(boundedRange);
            }
            updateArrowPosition(plot);
        }
    );

    QCPTextElement *valueLabel = new QCPTextElement(plot, "CPU, 0.0%", font);
    valueLabel->setTextFlags(Qt::AlignCenter); 

    int newRowIndex = plot->plotLayout()->rowCount();
    plot->plotLayout()->insertRow(newRowIndex);
    plot->plotLayout()->addElement(newRowIndex, 0, valueLabel);

    plot->setProperty("valueLabel", QVariant::fromValue(valueLabel));

    QCPItemText *arrowMarker = new QCPItemText(plot);
    arrowMarker->setColor(getGraphColor(index));
    arrowMarker->setFont(font);
    arrowMarker->setText("⯇ 0.0%");
    arrowMarker->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    arrowMarker->setClipToAxisRect(false); 

    arrowMarker->position->setTypeX(QCPItemPosition::ptAbsolute);
    arrowMarker->position->setTypeY(QCPItemPosition::ptPlotCoords);
    arrowMarker->position->setAxisRect(plot->axisRect());
    arrowMarker->position->setAxes(nullptr, plot->yAxis);

    arrowMarker->setVisible(false);
    plot->setProperty("arrowMarker", QVariant::fromValue(arrowMarker));
}

void CpuTabsWidget::onConnectionLost() {
    m_wasDisconnected = true; 
}

QColor CpuTabsWidget::getGraphColor(int index) {
    if (index == -1) return Qt::blue;
    static constexpr std::array<Qt::GlobalColor, 8> colors  = {
        Qt::red, Qt::green, Qt::yellow, Qt::magenta,
        Qt::black, Qt::gray, Qt::cyan, Qt::darkBlue
    };
    return colors[static_cast<size_t>(index) % colors.size()];
}
