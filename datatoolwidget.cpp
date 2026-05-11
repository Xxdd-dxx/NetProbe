#include "datatoolwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QClipboard>
#include <QApplication>
#include <QGroupBox>
#include <QFormLayout>
#include <QCryptographicHash>

DataToolWidget::DataToolWidget(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void DataToolWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QTabWidget *subTabs = new QTabWidget;

    // 创建两个 Tab
    QWidget *tabConvert = new QWidget;
    QWidget *tabHash = new QWidget;

    setupConvertTab(tabConvert);
    setupHashTab(tabHash);

    subTabs->addTab(tabConvert, "数据转换");
    subTabs->addTab(tabHash, "数据校验");

    mainLayout->addWidget(subTabs);
}

// ==========================================
// Tab 1: 数据转换界面布局
// ==========================================
void DataToolWidget::setupConvertTab(QWidget *tab)
{
    QVBoxLayout *convertLayout = new QVBoxLayout(tab);

    QHBoxLayout *typeLayout = new QHBoxLayout;
    typeLayout->addWidget(new QLabel("转换类型:"));
    typeCombo = new QComboBox;
    typeCombo->addItems({"十进制 -> 二进制", "二进制 -> 十进制", "字符串 -> 16进制", "16进制 -> 字符串"});
    typeLayout->addWidget(typeCombo);
    typeLayout->addStretch();
    convertLayout->addLayout(typeLayout);

    QHBoxLayout *inputHeader = new QHBoxLayout;
    inputHeader->addWidget(new QLabel("输入数据:"));
    inputHeader->addStretch();
    lblInputCount = new QLabel("字符数: 0, 字节数: 0");
    inputHeader->addWidget(lblInputCount);
    convertLayout->addLayout(inputHeader);

    inputEdit = new QTextEdit;
    inputEdit->setPlaceholderText("请输入要转换的数据...");
    convertLayout->addWidget(inputEdit);

    QHBoxLayout *outputHeader = new QHBoxLayout;
    outputHeader->addWidget(new QLabel("输出数据:"));
    outputHeader->addStretch();
    lblOutputCount = new QLabel("字符数: 0, 字节数: 0");
    outputHeader->addWidget(lblOutputCount);
    convertLayout->addLayout(outputHeader);

    outputEdit = new QTextEdit;
    outputEdit->setPlaceholderText("转换结果将显示在这里...");
    outputEdit->setReadOnly(true);
    convertLayout->addWidget(outputEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *btnConvert = new QPushButton("转换操作");
    QPushButton *btnClear = new QPushButton("清空数据");
    QPushButton *btnSwap = new QPushButton("交换输入输出");
    QPushButton *btnCopy = new QPushButton("复制结果");

    QString btnStyle = "background-color: #e5e7eb; padding: 10px;";
    btnConvert->setStyleSheet(btnStyle); btnClear->setStyleSheet(btnStyle);
    btnSwap->setStyleSheet(btnStyle); btnCopy->setStyleSheet(btnStyle);

    btnLayout->addWidget(btnConvert); btnLayout->addWidget(btnClear);
    btnLayout->addWidget(btnSwap); btnLayout->addWidget(btnCopy);
    convertLayout->addLayout(btnLayout);

    connect(btnConvert, &QPushButton::clicked, this, &DataToolWidget::onConvert);
    connect(btnClear, &QPushButton::clicked, this, &DataToolWidget::onClear);
    connect(btnSwap, &QPushButton::clicked, this, &DataToolWidget::onSwap);
    connect(btnCopy, &QPushButton::clicked, this, &DataToolWidget::onCopy);
    connect(inputEdit, &QTextEdit::textChanged, this, &DataToolWidget::onInputTextChanged);
}

// ==========================================
// Tab 2: 数据校验界面布局 (根据截图还原)
// ==========================================
void DataToolWidget::setupHashTab(QWidget *tab)
{
    QVBoxLayout *hashLayout = new QVBoxLayout(tab);

    // 上半部分：输入区域
    QGroupBox *grpInput = new QGroupBox("输入数据");
    QVBoxLayout *grpInputLayout = new QVBoxLayout(grpInput);

    inputHashEdit = new QTextEdit;
    inputHashEdit->setText("12345");
    grpInputLayout->addWidget(inputHashEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    QPushButton *btnCalc = new QPushButton("计算校验值");
    QPushButton *btnCopyAll = new QPushButton("复制所有校验值");
    chkUppercase = new QCheckBox("大写显示");
    chkUppercase->setChecked(false); // 默认小写

    QString btnStyle = "background-color: #e5e7eb; padding: 8px;";
    btnCalc->setStyleSheet(btnStyle);
    btnCopyAll->setStyleSheet(btnStyle);

    btnLayout->addWidget(btnCalc);
    btnLayout->addWidget(btnCopyAll);
    btnLayout->addWidget(chkUppercase);
    btnLayout->addStretch();
    grpInputLayout->addLayout(btnLayout);

    hashLayout->addWidget(grpInput);

    // 下半部分：输出区域
    QGroupBox *grpOutput = new QGroupBox("校验值结果");
    QFormLayout *formLayout = new QFormLayout(grpOutput);

    editCrc16 = new QLineEdit; editCrc16->setReadOnly(true);
    editCrc32 = new QLineEdit; editCrc32->setReadOnly(true);
    editMd5 = new QLineEdit;   editMd5->setReadOnly(true);
    editSha1 = new QLineEdit;  editSha1->setReadOnly(true);
    editSha256 = new QLineEdit; editSha256->setReadOnly(true); // 对应图里的CRC256

    formLayout->addRow("CRC16:", editCrc16);
    formLayout->addRow("CRC32:", editCrc32);
    formLayout->addRow("MD5:", editMd5);
    formLayout->addRow("SHA1:", editSha1);
    formLayout->addRow("CRC256 (实际为SHA256):", editSha256);

    hashLayout->addWidget(grpOutput);

    connect(btnCalc, &QPushButton::clicked, this, &DataToolWidget::onCalcHash);
    connect(btnCopyAll, &QPushButton::clicked, this, &DataToolWidget::onCopyAllHash);
    connect(chkUppercase, &QCheckBox::toggled, this, &DataToolWidget::onHashUppercaseChanged);

    // 默认计算一次
    onCalcHash();
}

// ==========================================
// 数据转换核心逻辑 (已彻底重构和修复)
// ==========================================
void DataToolWidget::onInputTextChanged() {
    QString txt = inputEdit->toPlainText();
    lblInputCount->setText(QString("字符数: %1, 字节数: %2").arg(txt.length()).arg(txt.toUtf8().length()));
}

void DataToolWidget::onConvert() {
    // 获取输入并去除首尾空格
    QString input = inputEdit->toPlainText().trimmed();
    QString output;
    bool ok;
    int type = typeCombo->currentIndex();

    if (input.isEmpty()) {
        outputEdit->clear();
        return;
    }

    if (type == 0) {
        // 十进制 -> 二进制 (使用 qulonglong 支持更大数值)
        qulonglong val = input.toULongLong(&ok, 10);
        if (ok) output = QString::number(val, 2);
        else output = "【错误】请输入有效的纯十进制数字，不要包含字母或特殊符号。";
    }
    else if (type == 1) {
        // 二进制 -> 十进制
        // 先去除可能存在的空格，防止带空格的二进制串报错
        QString cleanBin = input.replace(" ", "");
        qulonglong val = cleanBin.toULongLong(&ok, 2);
        if (ok) output = QString::number(val, 10);
        else output = "【错误】请输入有效的二进制数字（仅限 0 和 1）。";
    }
    else if (type == 2) {
        // 字符串 -> 16进制
        QByteArray hex = input.toUtf8().toHex();
        // 为了美观，手动每两个字符插入一个空格，兼容所有 Qt5 版本
        for (int i = 0; i < hex.length(); i += 2) {
            output += hex.mid(i, 2) + " ";
        }
        output = output.trimmed().toUpper();
    }
    else if (type == 3) {
        // 16进制 -> 字符串
        // 去除所有的空格和换行符，QByteArray::fromHex 才能准确识别
        QString cleanHex = input.replace(" ", "").replace("\n", "").replace("\r", "");
        QByteArray bytes = QByteArray::fromHex(cleanHex.toUtf8());
        output = QString::fromUtf8(bytes);
        if (output.isEmpty()) {
            output = "【错误】解析失败，请检查是否是有效的十六进制数据格式。";
        }
    }

    outputEdit->setPlainText(output);
    lblOutputCount->setText(QString("字符数: %1, 字节数: %2").arg(output.length()).arg(output.toUtf8().length()));
}

void DataToolWidget::onClear() { inputEdit->clear(); outputEdit->clear(); }
void DataToolWidget::onSwap() {
    QString t = inputEdit->toPlainText();
    inputEdit->setPlainText(outputEdit->toPlainText());
    outputEdit->setPlainText(t);
}
void DataToolWidget::onCopy() { QApplication::clipboard()->setText(outputEdit->toPlainText()); }


// ==========================================
// 数据校验核心逻辑
// ==========================================
quint16 DataToolWidget::calcCrc16(const QByteArray &data) {
    // 经典 Modbus CRC16 实现
    quint16 crc = 0xFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= (quint8)data.at(i);
        for (int j = 0; j < 8; ++j) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

quint32 DataToolWidget::calcCrc32(const QByteArray &data) {
    // 标准 IEEE 802.3 CRC32 实现
    quint32 crc = 0xFFFFFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= (quint8)data.at(i);
        for (int j = 0; j < 8; ++j) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFF;
}

void DataToolWidget::onCalcHash()
{
    QByteArray data = inputHashEdit->toPlainText().toUtf8();
    bool isUpper = chkUppercase->isChecked();

    // 计算 CRC
    quint16 crc16_val = calcCrc16(data);
    QString crc16_str = QString("%1").arg(crc16_val, 4, 16, QChar('0'));
    editCrc16->setText(isUpper ? crc16_str.toUpper() : crc16_str.toLower());

    quint32 crc32_val = calcCrc32(data);
    QString crc32_str = QString("%1").arg(crc32_val, 8, 16, QChar('0'));
    editCrc32->setText(isUpper ? crc32_str.toUpper() : crc32_str.toLower());

    // 使用 QCryptographicHash 计算 MD5, SHA1, SHA256
    QString md5 = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
    editMd5->setText(isUpper ? md5.toUpper() : md5.toLower());

    QString sha1 = QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex();
    editSha1->setText(isUpper ? sha1.toUpper() : sha1.toLower());

    // 原图写的是 CRC256，但实际上是 SHA-256 哈希值
    QString sha256 = QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
    editSha256->setText(isUpper ? sha256.toUpper() : sha256.toLower());
}

void DataToolWidget::onHashUppercaseChanged()
{
    // 复选框改变时，直接重新计算并刷新大小写即可
    onCalcHash();
}

void DataToolWidget::onCopyAllHash()
{
    QString text = QString("CRC16: %1\nCRC32: %2\nMD5: %3\nSHA1: %4\nSHA256: %5")
    .arg(editCrc16->text())
        .arg(editCrc32->text())
        .arg(editMd5->text())
        .arg(editSha1->text())
        .arg(editSha256->text());
    QApplication::clipboard()->setText(text);
}