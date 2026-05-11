#include "udpclientwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QDateTime>
#include <QNetworkInterface>

UdpClientWidget::UdpClientWidget(QWidget *parent) : QWidget(parent), udpClient(new QUdpSocket(this))
{
    setupUI();
    connect(udpClient, &QUdpSocket::readyRead, this, &UdpClientWidget::onReadyRead);
}

QStringList UdpClientWidget::getLocalIPs() {
    QStringList ips = {"127.0.0.1"};
    for (const auto &addr : QNetworkInterface::allAddresses()) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol && addr != QHostAddress::LocalHost) ips << addr.toString();
    }
    return ips;
}

void UdpClientWidget::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel("UDP客户端消息列表:"));
    logBrowser = new QTextBrowser;
    logBrowser->append("Prompt: Please enter data and click to send test.\n");
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
    portSpin->setValue(9999);
    formLayout->addRow("远程主机地址:", ipCombo);
    formLayout->addRow("远程主机端口:", portSpin);
    rightLayout->addWidget(grpParam);
    rightLayout->addStretch();

    QGroupBox *grpTest = new QGroupBox("测试消息:");
    QVBoxLayout *testLayout = new QVBoxLayout(grpTest);
    inputEdit = new QTextEdit;
    inputEdit->setText("Hello Udp Server.");
    inputEdit->setMaximumHeight(100);
    QPushButton *btnSend = new QPushButton("发送消息");
    btnSend->setStyleSheet("background-color: #3b82f6; color: white; padding: 5px;");

    QHBoxLayout *botLayout = new QHBoxLayout;
    botLayout->addStretch();
    botLayout->addWidget(btnSend);

    testLayout->addWidget(inputEdit);
    testLayout->addLayout(botLayout);
    rightLayout->addWidget(grpTest);
    mainLayout->addLayout(rightLayout, 3);

    connect(btnSend, &QPushButton::clicked, this, &UdpClientWidget::onSend);
}

void UdpClientWidget::logMessage(const QString &prefix, const QString &msg) {
    QString time = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
    logBrowser->append(QString("[%1] %2\n%3\n").arg(time).arg(prefix).arg(msg));
}

void UdpClientWidget::onSend() {
    QString msg = inputEdit->toPlainText();
    udpClient->writeDatagram(msg.toUtf8(), QHostAddress(ipCombo->currentText()), portSpin->value());
    logMessage("发送数据", msg);
}

void UdpClientWidget::onReadyRead() {
    while (udpClient->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpClient->pendingDatagramSize());
        udpClient->readDatagram(datagram.data(), datagram.size());
        logMessage("接收数据", QString::fromUtf8(datagram));
    }
}