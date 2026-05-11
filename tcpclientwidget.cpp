#include "tcpclientwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QDateTime>
#include <QNetworkInterface>
#include <QMessageBox>

TcpClientWidget::TcpClientWidget(QWidget *parent) : QWidget(parent),
    tcpClient(new QTcpSocket(this)),
    connectTimer(new QTimer(this))
{
    setupUI();

    // 绑一下信号和槽
    connect(tcpClient, &QTcpSocket::connected, this, &TcpClientWidget::onConnected);
    connect(tcpClient, &QTcpSocket::disconnected, this, &TcpClientWidget::onDisconnected);
    connect(tcpClient, &QIODevice::readyRead, this, &TcpClientWidget::onReadyRead);

    // 处理报错，顺便兼容一下旧版本Qt的写法
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(tcpClient, &QTcpSocket::errorOccurred, this, &TcpClientWidget::onError);
#else
    connect(tcpClient, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), this, &TcpClientWidget::onError);
#endif

    // 单次触发的定时器
    connectTimer->setSingleShot(true);
    connect(connectTimer, &QTimer::timeout, this, &TcpClientWidget::onConnectTimeout);
}

// 查一下本机的IP
QStringList TcpClientWidget::getLocalIPs() {
    QStringList ips = {"127.0.0.1"};
    for (const auto &addr : QNetworkInterface::allAddresses()) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol && addr != QHostAddress::LocalHost) {
            ips << addr.toString();
        }
    }
    return ips;
}

void TcpClientWidget::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel("TCP客户端消息列表:"));
    logBrowser = new QTextBrowser;
    leftLayout->addWidget(logBrowser);
    mainLayout->addLayout(leftLayout, 7);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    QGroupBox *grpParam = new QGroupBox("参数设置:");
    QFormLayout *formLayout = new QFormLayout(grpParam);
    ipCombo = new QComboBox;
    ipCombo->setEditable(true);
    ipCombo->addItems(getLocalIPs());
    portSpin = new QSpinBox;
    portSpin->setRange(1, 65535);
    portSpin->setValue(12345);
    formLayout->addRow("远程主机地址:", ipCombo);
    formLayout->addRow("远程主机端口:", portSpin);
    rightLayout->addWidget(grpParam);

    btnConnect = new QPushButton("连接服务器");
    btnConnect->setStyleSheet("background-color: #a3e635; color: black; padding: 5px;");
    btnDisconnect = new QPushButton("断开服务器");
    btnDisconnect->setStyleSheet("background-color: #3b82f6; color: white; padding: 5px;");
    btnDisconnect->setEnabled(false);
    QPushButton *btnExit = new QPushButton("退出程序");
    btnExit->setStyleSheet("background-color: #3b82f6; color: white; padding: 5px;");
    connect(btnExit, &QPushButton::clicked, window(), &QWidget::close);

    rightLayout->addWidget(btnConnect);
    rightLayout->addWidget(btnDisconnect);
    rightLayout->addWidget(btnExit);
    rightLayout->addStretch();

    QGroupBox *grpTest = new QGroupBox("测试消息:");
    QVBoxLayout *testLayout = new QVBoxLayout(grpTest);
    inputEdit = new QTextEdit;
    inputEdit->setText("Hello TCP Server.");
    inputEdit->setMaximumHeight(100);

    QHBoxLayout *botLayout = new QHBoxLayout;
    QPushButton *btnSend = new QPushButton("发送消息");
    btnSend->setStyleSheet("background-color: #3b82f6; color: white; padding: 5px;");

    // 把按钮挤到右边去
    //botLayout->addStretch();
    botLayout->addWidget(btnSend);

    testLayout->addWidget(inputEdit);
    testLayout->addLayout(botLayout);
    rightLayout->addWidget(grpTest);
    mainLayout->addLayout(rightLayout, 3);

    connect(btnConnect, &QPushButton::clicked, this, &TcpClientWidget::onConnect);
    connect(btnDisconnect, &QPushButton::clicked, this, &TcpClientWidget::onDisconnect);
    connect(btnSend, &QPushButton::clicked, this, &TcpClientWidget::onSend);
}

// 打印日志用的封装函数
void TcpClientWidget::logMessage(const QString &prefix, const QString &msg) {
    QString time = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
    logBrowser->append(QString("[%1] %2\n%3\n").arg(time).arg(prefix).arg(msg));
}

void TcpClientWidget::onConnect() {
    // 点了连接后把按钮灰掉，免得用户狂点出bug
    btnConnect->setEnabled(false);
    btnDisconnect->setEnabled(true);

    logMessage("系统消息", QString("正在连接到 %1:%2...").arg(ipCombo->currentText()).arg(portSpin->value()));

    tcpClient->connectToHost(ipCombo->currentText(), portSpin->value());

    // 定时器设为3秒，如果3秒还没连上就触发超时
    connectTimer->start(3000);
}

void TcpClientWidget::onDisconnect() {
    // 如果还没连上用户就手贱点断开，得把定时器关了
    if (connectTimer->isActive()) {
        connectTimer->stop();
    }
    // 直接用abort比较干脆，能马上断掉
    tcpClient->abort();
}

void TcpClientWidget::onConnected() {
    // 成功连上啦，关掉定时器
    if (connectTimer->isActive()) {
        connectTimer->stop();
    }
    logMessage("系统消息", "连接服务器成功");
    btnConnect->setEnabled(false);
    btnDisconnect->setEnabled(true);
}

void TcpClientWidget::onConnectTimeout() {
    // 超时了，强制断开
    tcpClient->abort();
    logMessage("系统错误", "连接服务器超时！请检查目标IP、端口或网络状况。");

    btnConnect->setEnabled(true);
    btnDisconnect->setEnabled(false);
}

void TcpClientWidget::onError(QAbstractSocket::SocketError socketError) {
    // 网络底层报错了，提前结束等待，停掉定时器
    if (connectTimer->isActive()) {
        connectTimer->stop();
    }

    // 服务器主动踢掉的不用在这里报，不然会报两次
    if (socketError != QAbstractSocket::RemoteHostClosedError) {
        logMessage("系统错误", "连接失败: " + tcpClient->errorString());
    }

    btnConnect->setEnabled(true);
    btnDisconnect->setEnabled(false);
}

void TcpClientWidget::onDisconnected() {
    logMessage("系统消息", "已断开与服务器的连接");
    btnConnect->setEnabled(true);
    btnDisconnect->setEnabled(false);
}

void TcpClientWidget::onReadyRead() {
    logMessage("接收数据", QString::fromUtf8(tcpClient->readAll()));
}

void TcpClientWidget::onSend() {
    // 判断一下到底连上没有才能发
    if(tcpClient->state() == QAbstractSocket::ConnectedState) {
        QString msg = inputEdit->toPlainText();
        tcpClient->write(msg.toUtf8());
        logMessage("发送数据", msg);
    } else {
        QMessageBox::warning(this, "警告", "未连接到服务器！");
    }
}