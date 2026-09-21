#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QWidget>
#include <QMainWindow>

class AppCore;
class QStackedWidget;
class CpuTabsWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void initConnections(AppCore *core);

private:
    QStackedWidget *m_stackedWidget;
    CpuTabsWidget *m_tabsWidget;
};
#endif