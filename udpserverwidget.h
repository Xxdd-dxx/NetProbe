#ifndef UDPSERVERWIDGET_H
#define UDPSERVERWIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QTextBrowser>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTextEdit>

class UdpServerWidget : public QWidget
{
    Q_OBJECT
    public:
             explicit UdpServerWidget(QWidget *parent = nullptr);
private slots:
    void onStart();
    void onStop();
    void onSend();
    void onReadyRead();
private:
    void setupUI();
    void logMessage(const QString &prefix, const QString &msg);
    QStringList getLocalIPs();

    QUdpSocket *udpServer;
    QHostAddress lastSenderIp;
    quint16 lastSenderPort;

    QTextBrowser *logBrowser;
    QComboBox *ipCombo;
    QSpinBox *portSpin;
    QPushButton *btnStart;
    QPushButton *btnStop;
    QTextEdit *inputEdit;
};
#endif // UDPSERVERWIDGET_H