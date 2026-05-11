#ifndef DATATOOLWIDGET_H
#define DATATOOLWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>

class DataToolWidget : public QWidget
{
    Q_OBJECT
    public:
             explicit DataToolWidget(QWidget *parent = nullptr);

private slots:
    // 数据转换槽函数
    void onConvert();
    void onClear();
    void onSwap();
    void onCopy();
    void onInputTextChanged();

    // 数据校验槽函数
    void onCalcHash();
    void onCopyAllHash();
    void onHashUppercaseChanged();

private:
    void setupUI();
    void setupConvertTab(QWidget *tab);
    void setupHashTab(QWidget *tab);

    // CRC计算辅助函数
    quint16 calcCrc16(const QByteArray &data);
    quint32 calcCrc32(const QByteArray &data);

    // --- 数据转换 UI ---
    QComboBox *typeCombo;
    QTextEdit *inputEdit;
    QTextEdit *outputEdit;
    QLabel *lblInputCount;
    QLabel *lblOutputCount;

    // --- 数据校验 UI ---
    QTextEdit *inputHashEdit;
    QCheckBox *chkUppercase;
    QLineEdit *editCrc16;
    QLineEdit *editCrc32;
    QLineEdit *editMd5;
    QLineEdit *editSha1;
    QLineEdit *editSha256;
};

#endif // DATATOOLWIDGET_H