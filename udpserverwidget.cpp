#include "udpserverwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QDateTime>
#include <QNetworkInterface>
#include <QMessageBox>

UdpServerWidget::UdpServerWidget(QWidget *parent) : QWidget(parent), udpServer(new QUdpSocket(this)), lastSenderPort(0)
{
    setupUI();
    connect(udpServer, &QUdpSocket::readyRead, this, &UdpServerWidget::onReadyRead);
}

QStringList UdpServerWidget::getLocalIPs() {
    QStringList ips = {"127.0.0.1"};
    for (const auto &addr : QNetworkInterface::allAddresses()) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol && addr != QHostAddress::LocalHost) ips << addr.toString();
    }
    ips << "0.0.0.0";
    return ips;
}

void UdpServerWidget::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel("UDP服务器消息列表:"));
    logBrowser = new QTextBrowser;
    leftLayout->addWidget(logBrowser);
    mainLayout->addLayout(leftLayout, 7);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    QGroupBox *grpParam = new QGroupBox("参数设置:");
    QFormLayout *formLayout = new QFormLayout(grpParam);
    ipCombo = new QComboBox;
    ipCombo->addItems(getLocalIPs());
    portSpin = new QSpinBox;
    portSpin->setRange(1, 65535);
    portSpin->setValue(9999);
    formLayout->addRow("本地主机地址:", ipCombo);
    formLayout->addRow("本地主机端口:", portSpin);
    rightLayout->addWidget(grpParam);

    btnStart = new QPushButton("启动监听");
    btnStart->setStyleSheet("background-color: #3b82f6; color: white; padding: 5px;");
    btnStop = new QPushButton("关闭监听");
    btnStop->setEnabled(false);
    QPushButton *btnExit = new QPushButton("退出程序");
    btnExit->setStyleSheet("background-color: #ef4444; color: white; padding: 5px;");
    connect(btnExit, &QPushButton::clicked, window(), &QWidget::close);

    rightLayout->addWidget(btnStart);
    rightLayout->addWidget(btnStop);
    rightLayout->addWidget(btnExit);
    rightLayout->addStretch();

    QGroupBox *grpTest = new QGroupBox("测试消息:");
    QVBoxLayout *testLayout = new QVBoxLayout(grpTest);
    inputEdit = new QTextEdit;
    inputEdit->setText("Hello Udp Server.");
    inputEdit->setMaximumHeight(100);
    QPushButton *btnSend = new QPushButton("发送消息");
    btnSend->setStyleSheet("background-color: #3b82f6; color: white; padding: 5px;");
    testLayout->addWidget(inputEdit);
    testLayout->addWidget(btnSend);
    rightLayout->addWidget(grpTest);
    mainLayout->addLayout(rightLayout, 3);

    connect(btnStart, &QPushButton::clicked, this, &UdpServerWidget::onStart);
    connect(btnStop, &QPushButton::clicked, this, &UdpServerWidget::onStop);
    connect(btnSend, &QPushButton::clicked, this, &UdpServerWidget::onSend);
}

void UdpServerWidget::logMessage(const QString &prefix, const QString &msg) {
    QString time = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
    logBrowser->append(QString("[%1] %2\n%3\n").arg(time).arg(prefix).arg(msg));
}

void UdpServerWidget::onStart() {
    QHostAddress addr(ipCombo->currentText() == "0.0.0.0" ? "0.0.0.0" : ipCombo->currentText());
    if (udpServer->bind(addr, portSpin->value())) {
        logMessage("服务器状态", "UDP服务器启动监听成功");
        btnStart->setEnabled(false); btnStop->setEnabled(true);
    } else {
        QMessageBox::critical(this, "错误", "绑定失败: " + udpServer->errorString());
    }
}

void UdpServerWidget::onStop() {
    udpServer->close();
    logMessage("服务器状态", "UDP服务器停止监听");
    btnStart->setEnabled(true); btnStop->setEnabled(false);
}

void UdpServerWidget::onReadyRead() {
    while (udpServer->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpServer->pendingDatagramSize());
        udpServer->readDatagram(datagram.data(), datagram.size(), &lastSenderIp, &lastSenderPort);
        logMessage("接收数据", QString::fromUtf8(datagram));
    }
}

void UdpServerWidget::onSend() {
    if (lastSenderPort == 0) {
        QMessageBox::warning(this, "警告", "尚无客户端发送数据，不知道目标地址！");
        return;
    }
    QString msg = inputEdit->toPlainText();
    udpServer->writeDatagram(msg.toUtf8(), lastSenderIp, lastSenderPort);
    logMessage("发送数据", msg);
}