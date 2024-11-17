#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>
#include <QComboBox>
#include <thread>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,m_p_SendSerialPortManager(std::make_unique<CSerialPortManager>())
    ,m_p_RecSerialPortManager(std::make_unique<CSerialPortManager>())
{
    ui->setupUi(this);
    init();


    connect(m_p_RecSerialPortManager.get(),&CSerialPortManager::signal_DataReceived,this,&MainWindow::handleDataReceived);
    connect(m_p_SendSerialPortManager.get(),&CSerialPortManager::signal_ErrorOccurred,this,&MainWindow::handleSerialportError);
    connect(m_p_RecSerialPortManager.get(),&CSerialPortManager::signal_ErrorOccurred,this,&MainWindow::handleSerialportError);
}

MainWindow::~MainWindow()
{
    // 关闭发送串口
    if (m_p_SendSerialPortManager->isOpen()) {
        m_p_SendSerialPortManager->closePort();
        m_p_SendSerialPortManager.reset();
    }

    // 关闭接收串口
    if (m_p_RecSerialPortManager->isOpen()) {
        m_p_RecSerialPortManager->closePort();
        m_p_RecSerialPortManager.reset();
    }
    delete ui;
}

void MainWindow::init()
{
    //获取所有的可用的接口
    auto portInfos=QSerialPortInfo::availablePorts();
    for (auto & portInfo : portInfos) {
        ui->comboBox_ChoseSendPort->addItem(portInfo.portName()+":"+portInfo.description(),portInfo.portName());
        ui->comboBox_ChoseRecPort->addItem(portInfo.portName()+":"+portInfo.description(),portInfo.portName());
    }

    //获取标准的波特率
    auto baudRates=QSerialPortInfo::standardBaudRates();
    for(auto &baudRate:baudRates){
        ui->comboBox_ChoseSendBaudRate->addItem(QString::number(baudRate),baudRate);
        ui->comboBox_ChoseRecBaudRate->addItem(QString::number(baudRate),baudRate);
    }
    ui->comboBox_ChoseSendBaudRate->setCurrentText("9600");
    ui->comboBox_ChoseRecBaudRate->setCurrentText("9600");

    //设置停止位
    ui->comboBox_ChoseSendStopBits->addItem("1",QSerialPort::OneStop);
    ui->comboBox_ChoseSendStopBits->addItem("1.5",QSerialPort::OneAndHalfStop);
    ui->comboBox_ChoseSendStopBits->addItem("2",QSerialPort::TwoStop);
    ui->comboBox_ChoseSendStopBits->setCurrentText("1");

    ui->comboBox_ChoseRecStopBits->addItem("1",QSerialPort::OneStop);
    ui->comboBox_ChoseRecStopBits->addItem("1.5",QSerialPort::OneAndHalfStop);
    ui->comboBox_ChoseRecStopBits->addItem("2",QSerialPort::TwoStop);
    ui->comboBox_ChoseRecStopBits->setCurrentText("1");

    //设置数据位
    ui->comboBox_ChoseSendDataBits->addItem("5",QSerialPort::Data5);
    ui->comboBox_ChoseSendDataBits->addItem("6",QSerialPort::Data6);
    ui->comboBox_ChoseSendDataBits->addItem("7",QSerialPort::Data7);
    ui->comboBox_ChoseSendDataBits->addItem("8",QSerialPort::Data8);
    ui->comboBox_ChoseSendDataBits->setCurrentText("8");

    ui->comboBox_ChoseRecDataBits->addItem("5",QSerialPort::Data5);
    ui->comboBox_ChoseRecDataBits->addItem("6",QSerialPort::Data6);
    ui->comboBox_ChoseRecDataBits->addItem("7",QSerialPort::Data7);
    ui->comboBox_ChoseRecDataBits->addItem("8",QSerialPort::Data8);
    ui->comboBox_ChoseRecDataBits->setCurrentText("8");

    //设置校验位
    ui->comboBox_ChoseSendParityBits->addItem("None",QSerialPort::NoParity);
    ui->comboBox_ChoseSendParityBits->addItem("Even",QSerialPort::EvenParity);
    ui->comboBox_ChoseSendParityBits->addItem("Odd",QSerialPort::OddParity);
    ui->comboBox_ChoseSendParityBits->addItem("Space",QSerialPort::SpaceParity);
    ui->comboBox_ChoseSendParityBits->addItem("Mark",QSerialPort::MarkParity);
    ui->comboBox_ChoseSendParityBits->setCurrentText("None");

    ui->comboBox_ChoseRecParityBits->addItem("None",QSerialPort::NoParity);
    ui->comboBox_ChoseRecParityBits->addItem("Even",QSerialPort::EvenParity);
    ui->comboBox_ChoseRecParityBits->addItem("Odd",QSerialPort::OddParity);
    ui->comboBox_ChoseRecParityBits->addItem("Space",QSerialPort::SpaceParity);
    ui->comboBox_ChoseRecParityBits->addItem("Mark",QSerialPort::MarkParity);
    ui->comboBox_ChoseRecParityBits->setCurrentText("None");

}

void MainWindow::updateUIOnPortChange(QPushButton *button, bool isPortOpen)
{
    if(isPortOpen)
    {
        button->setText("关闭串口");
    }else{
        button->setText("打开串口");
    }

}

void MainWindow::on_pushButton_OpenSendPort_clicked()
{
    if(m_p_SendSerialPortManager->isOpen())
    {
        m_p_SendSerialPortManager->closePort();
        updateUIOnPortChange(ui->pushButton_OpenSendPort,false);

        // 等待关闭完成后再尝试重新打开串口
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 等待500ms
    }else{
        QString portName=ui->comboBox_ChoseSendPort->currentData().toString();
        int baudRate=ui->comboBox_ChoseSendBaudRate->currentData().toInt();
        int dataBits=ui->comboBox_ChoseSendDataBits->currentData().toInt();
        int parity=ui->comboBox_ChoseSendParityBits->currentData().toInt();
        int stopBits=ui->comboBox_ChoseSendStopBits->currentData().toInt();

        if(m_p_SendSerialPortManager->openPort(portName,baudRate,dataBits,parity,stopBits))
        {
            updateUIOnPortChange(ui->pushButton_OpenSendPort,true);
        }else
        {
            QMessageBox::warning(this,"警告","打开串口失败");
        }
    }
}


void MainWindow::on_pushButton_OpenRecPort_clicked()
{
    if(m_p_RecSerialPortManager->isOpen())
    {
        m_p_RecSerialPortManager->closePort();
        updateUIOnPortChange(ui->pushButton_OpenRecPort,false);
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 等待500ms
    }else{
        QString portName=ui->comboBox_ChoseRecPort->currentData().toString();
        int baudRate=ui->comboBox_ChoseRecBaudRate->currentData().toInt();
        int dataBits=ui->comboBox_ChoseRecDataBits->currentData().toInt();
        int parity=ui->comboBox_ChoseRecParityBits->currentData().toInt();
        int stopBits=ui->comboBox_ChoseRecStopBits->currentData().toInt();

        if(m_p_RecSerialPortManager->openPort(portName,baudRate,dataBits,parity,stopBits))
        {
            updateUIOnPortChange(ui->pushButton_OpenRecPort,true);
        }else{
            QMessageBox::warning(this,"警告","打开串口失败");
        }
    }
}


void MainWindow::on_pushButton_Send_clicked()
{
    if(!m_p_SendSerialPortManager->isOpen())
    {
        QMessageBox::warning(this,"警告","请先打开发送串口");
        return;
    }
    QByteArray data=ui->plainTextEdit_SendMessage->toPlainText().toUtf8();
    m_p_SendSerialPortManager->sendData(data);
}


void MainWindow::on_pushButton_Clean_clicked()
{
    ui->plainTextEdit_SendMessage->clear();
}

void MainWindow::handleDataReceived(const QByteArray &data)
{
    ui->plainTextEdit_RecMessage->appendPlainText(QString::fromUtf8(data));
}

void MainWindow::handleSerialportError(const QString &error)
{
    ui->plainTextEdit_ErrorMessage->appendPlainText(error);
}

