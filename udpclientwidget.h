#ifndef UDPCLIENTWIDGET_H
#define UDPCLIENTWIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QTextBrowser>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTextEdit>

class UdpClientWidget : public QWidget
{
    Q_OBJECT
    public:
             explicit UdpClientWidget(QWidget *parent = nullptr);
private slots:
    void onSend();
    void onReadyRead();
private:
    void setupUI();
    void logMessage(const QString &prefix, const QString &msg);
    QStringList getLocalIPs();

    QUdpSocket *udpClient;
    QTextBrowser *logBrowser;
    QComboBox *ipCombo;
    QSpinBox *portSpin;
    QTextEdit *inputEdit;
};
#endif // UDPCLIENTWIDGET_H