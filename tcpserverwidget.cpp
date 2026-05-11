#include "tcpserverwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QDateTime>
#include <QNetworkInterface>
#include <QMessageBox>

TcpServerWidget::TcpServerWidget(QWidget *parent) : QWidget(parent), tcpServer(new QTcpServer(this))
{
    setupUI();
    connect(tcpServer, &QTcpServer::newConnection, this, &TcpServerWidget::onNewConnection);
}

QStringList TcpServerWidget::getLocalIPs() {
    QStringList ips = {"127.0.0.1"};
    for (const QHostAddress &addr : QNetworkInterface::allAddresses()) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol && addr != QHostAddress::LocalHost) {
            ips << addr.toString();
        }
    }
    ips << "0.0.0.0";
    return ips;
}

void TcpServerWidget::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel("TCP服务器消息列表:"));
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
    portSpin->setValue(12345);
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
    inputEdit->setText("Hello TCP Client.");
    inputEdit->setMaximumHeight(100);
    QPushButton *btnSend = new QPushButton("发送消息");
    btnSend->setStyleSheet("background-color: #3b82f6; color: white; padding: 5px;");
    testLayout->addWidget(inputEdit);
    testLayout->addWidget(btnSend);
    rightLayout->addWidget(grpTest);
    mainLayout->addLayout(rightLayout, 3);

    connect(btnStart, &QPushButton::clicked, this, &TcpServerWidget::onStart);
    connect(btnStop, &QPushButton::clicked, this, &TcpServerWidget::onStop);
    connect(btnSend, &QPushButton::clicked, this, &TcpServerWidget::onSend);
}

void TcpServerWidget::logMessage(const QString &prefix, const QString &msg) {
    QString time = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
    logBrowser->append(QString("[%1] %2\n%3\n").arg(time).arg(prefix).arg(msg));
}

void TcpServerWidget::onStart() {
    QHostAddress addr(ipCombo->currentText() == "0.0.0.0" ? "0.0.0.0" : ipCombo->currentText());
    if (tcpServer->listen(addr, portSpin->value())) {
        logMessage("服务器状态", "TCP服务器启动监听成功");
        btnStart->setEnabled(false); btnStop->setEnabled(true);
    } else {
        QMessageBox::critical(this, "错误", "监听失败: " + tcpServer->errorString());
    }
}

void TcpServerWidget::onStop() {
    tcpServer->close();
    for(QTcpSocket* s : tcpClientList) s->disconnectFromHost();
    tcpClientList.clear();
    logMessage("服务器状态", "TCP服务器停止监听");
    btnStart->setEnabled(true); btnStop->setEnabled(false);
}

void TcpServerWidget::onNewConnection() {
    QTcpSocket *client = tcpServer->nextPendingConnection();
    tcpClientList.append(client);
    connect(client, &QIODevice::readyRead, this, &TcpServerWidget::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &TcpServerWidget::onClientDisconnected);
    logMessage("服务器状态", QString("客户端 %1:%2 已连接").arg(client->peerAddress().toString()).arg(client->peerPort()));
}

void TcpServerWidget::onReadyRead() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (client) logMessage("接收数据", QString::fromUtf8(client->readAll()));
}

void TcpServerWidget::onClientDisconnected() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        tcpClientList.removeOne(client);
        client->deleteLater();
        logMessage("服务器状态", "客户端已断开连接");
    }
}

void TcpServerWidget::onSend() {
    QString msg = inputEdit->toPlainText();
    if (msg.isEmpty() || tcpClientList.isEmpty()) return;
    QByteArray data = msg.toUtf8();
    for(QTcpSocket* s : tcpClientList) s->write(data);
    logMessage("发送数据", msg);
}