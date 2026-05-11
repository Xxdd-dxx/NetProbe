#include "mainwindow.h"
#include "tcpserverwidget.h"
#include "tcpclientwidget.h"
#include "udpserverwidget.h"
#include "udpclientwidget.h"
#include "datatoolwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("TCP/UDP协议网络调试助手");
    resize(900, 600);

    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    tabWidget->addTab(new TcpServerWidget(this), "TCP服务器");
    tabWidget->addTab(new TcpClientWidget(this), "TCP客户端");
    tabWidget->addTab(new UdpServerWidget(this), "UDP服务器");
    tabWidget->addTab(new UdpClientWidget(this), "UDP客户端");
    tabWidget->addTab(new DataToolWidget(this), "数据转换工具");
}

MainWindow::~MainWindow() {}