#ifndef TCPCLIENTWIDGET_H
#define TCPCLIENTWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QTextBrowser>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>

class TcpClientWidget : public QWidget
{
    Q_OBJECT
    public:
             explicit TcpClientWidget(QWidget *parent = nullptr);

private slots:
    void onConnect();
    void onDisconnect();
    void onSend();
    void onConnected();
    void onDisconnected();
    void onReadyRead();

    // 超时和报错的处理槽函数
    void onConnectTimeout();
    void onError(QAbstractSocket::SocketError socketError);

private:
    void setupUI();
    void logMessage(const QString &prefix, const QString &msg);
    QStringList getLocalIPs();

    QTcpSocket *tcpClient;
    QTimer *connectTimer; // 用来处理超时的定时器

    QTextBrowser *logBrowser;
    QComboBox *ipCombo;
    QSpinBox *portSpin;
    QPushButton *btnConnect;
    QPushButton *btnDisconnect;
    QTextEdit *inputEdit;
};

#endif // TCPCLIENTWIDGET_H