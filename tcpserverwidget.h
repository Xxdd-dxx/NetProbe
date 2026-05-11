#ifndef TCPSERVERWIDGET_H
#define TCPSERVERWIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextBrowser>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTextEdit>

class TcpServerWidget : public QWidget
{
     Q_OBJECT
    public:
             explicit TcpServerWidget(QWidget *parent = nullptr);
private slots:
    void onStart();
    void onStop();
    void onSend();
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();
private:
    void setupUI();
    void logMessage(const QString &prefix, const QString &msg);
    QStringList getLocalIPs();

    QTcpServer *tcpServer;
    QList<QTcpSocket*> tcpClientList;

    QTextBrowser *logBrowser;
    QComboBox *ipCombo;
    QSpinBox *portSpin;
    QPushButton *btnStart;
    QPushButton *btnStop;
    QTextEdit *inputEdit;
};
#endif // TCPSERVERWIDGET_H