#include "cserialportmanager.h"

#include <qdebug.h>
#include <thread>
#include <memory>
#include <atomic>
#include <boost/asio/io_context.hpp>
CSerialPortManager::CSerialPortManager(QObject *parent)
    : QObject{parent},m_IsPortOpen(false),m_IoContext(),m_p_SerialPort(nullptr)
{
    m_ReadBuffer.clear();


}

CSerialPortManager::~CSerialPortManager()
{
    closePort();

}

bool CSerialPortManager::openPort(const QString &portName, int baudRate, int dataBits, int parity, int stopBits)
{
    if(m_IsPortOpen)
    {
        emit signal_ErrorOccurred("Port is already open");
        return false;
    }
    try
    {
        std::string port = portName.toStdString();
        if (portName.startsWith("COM") && portName.mid(3).toInt() > 9) {
            port = "\\\\.\\" + port;
        }

        qDebug() << "Port Name: " << portName;
        qDebug() << "Baud Rate: " << baudRate;
        qDebug() << "Data Bits: " << dataBits;
        qDebug() << "Parity: " << parity;
        qDebug() << "Stop Bits: " << stopBits;

        m_p_SerialPort = std::make_unique<boost::asio::serial_port>(m_IoContext, portName.toStdString());
        m_p_SerialPort->set_option(boost::asio::serial_port::baud_rate(baudRate));
        m_p_SerialPort->set_option(boost::asio::serial_port::character_size(dataBits));
        // Set stop bits correctly, depending on the selected value
        if (stopBits == 1) {
            m_p_SerialPort->set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one));
        } else if (stopBits == 1.5) {
            m_p_SerialPort->set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::onepointfive));
        } else if (stopBits == 2) {
            m_p_SerialPort->set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::two));
        }
        m_p_SerialPort->set_option(boost::asio::serial_port::parity(static_cast<boost::asio::serial_port::parity::type>(parity)));

        //qDebug端口的参数


        qDebug() << "IoContext status: " << &m_IoContext;
        qDebug() << "Serial port object created: " << m_p_SerialPort.get();
        qDebug() << "Serial port opened successfully!";
        m_IsPortOpen.store(true);  // 标记串口为已打开状态

        // 启动异步任务来poll io_context
        m_AsyncPollThread = std::async(std::launch::async, [this]() {
            while (m_IsPortOpen.load()) {
                m_IoContext.poll(); // 持续poll，直到端口关闭
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 加一个微小的延迟，避免100% CPU占用
            }
        });

        readData();
        return true;
    }catch(const boost::system::system_error &e)
    {
        emit signal_ErrorOccurred(QString("Failed to open port: %1").arg(e.what()));
        qDebug() << QString("Failed to open port: %1").arg(e.what());
        return false;
    }
}
void CSerialPortManager::closePort()
{
    if (!m_IsPortOpen) {
        qDebug() << "Port is already closed!";
        return;
    }

    try {
        // 停止异步操作
        m_IoContext.stop();

        // 强制等待所有异步任务完成
        if (m_AsyncPollThread.valid()) {
            std::future_status status = m_AsyncPollThread.wait_for(std::chrono::milliseconds(2000));
            if (status == std::future_status::timeout) {
                qDebug() << "Async poll thread did not finish in time!";
            } else {
                qDebug() << "Async poll thread finished.";
            }
        }

        // 取消所有异步操作
        if (m_p_SerialPort) {
            boost::system::error_code ec;
            m_p_SerialPort->cancel(ec);  // 取消所有异步任务
            if (ec) {
                qDebug() << "Failed to cancel operations: " << ec.message().c_str();
            }
        }

        // 等待所有异步操作完成
        m_IoContext.restart();  // 重新启动 io_context
        m_IoContext.run();  // 强制执行所有挂起的异步操作，确保它们完成

        // 关闭串口
        boost::system::error_code ec;
        if (m_p_SerialPort) {
            m_p_SerialPort->close(ec);  // 关闭串口
        }

        if (ec) {
            qDebug() << "Failed to close port:" << ec.message().c_str();
            emit signal_ErrorOccurred(ec.message().c_str());
            return;
        }

        qDebug() << "Port successfully closed.";

        // 重置串口对象，彻底清理资源
        m_p_SerialPort.reset();  // 重置串口对象
        m_IsPortOpen.store(false);  // 更新状态
        emit signal_PortClosed();  // 发出信号

        // 额外的延时，确保资源释放
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 等待更多时间确保串口完全关闭

    } catch (const boost::system::system_error &e) {
        qDebug() << "System error while closing port:" << e.what();
        emit signal_ErrorOccurred(QString("Failed to close port: %1").arg(e.what()));
    }
}



bool CSerialPortManager::isOpen() const
{
    return m_IsPortOpen.load();
}

void CSerialPortManager::sendData(const QByteArray &data)
{
    if (!m_IsPortOpen.load()) {
        emit signal_ErrorOccurred("Port is not open.");
        return;
    }
    try {
        m_WriteBuffer.push_back(boost::asio::buffer(data.constData(),data.size()));
        if(m_WriteBuffer.size()==1)
        {
            boost::asio::async_write(*m_p_SerialPort,m_WriteBuffer.front(),
                                     std::bind(&CSerialPortManager::handleWrite,this,
                                               boost::asio::placeholders::error,
                                               boost::asio::placeholders::bytes_transferred));
        }
    } catch (const boost::system::system_error &e) {
        emit signal_ErrorOccurred(QString("Failed to send data: %1").arg(e.what()));
    }


}
void CSerialPortManager::readData()
{
    if (!m_IsPortOpen.load()) {
        emit signal_ErrorOccurred("Port is not open.");
        return;
    }
    m_ReadBuffer.resize(1024);
    m_p_SerialPort->async_read_some(boost::asio::buffer(m_ReadBuffer)
                                    ,std::bind(&CSerialPortManager::handleRead,this,
                                     boost::asio::placeholders::error,
                                     boost::asio::placeholders::bytes_transferred));

}

void CSerialPortManager::handleWrite(const boost::system::error_code &error, size_t bytesTransferred)
{
    if(error)
    {
        emit signal_ErrorOccurred(QString("Failed to write data: %1").arg(error.message().c_str()));
        return;
    }
    if (!m_WriteBuffer.empty()) {
        m_WriteBuffer.pop_front();
    }
    if (!m_WriteBuffer.empty()) {
        boost::asio::async_write(*m_p_SerialPort,m_WriteBuffer.front(),
                                 std::bind(&CSerialPortManager::handleWrite,this,
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred));
    }
}



void CSerialPortManager::handleRead(const boost::system::error_code &error, size_t bytesTransferred)
{
    if(error)
    {
        emit signal_ErrorOccurred(QString("Failed to read data: %1").arg(error.message().c_str()));
        return;
    }

    QByteArray receivedData(m_ReadBuffer.data(), bytesTransferred);

    // 将接收到的数据放入队列中，进行后续处理
    {
        m_ReceivedDataQueue.push(receivedData);
    }


    emit signal_DataReceived(receivedData);
    readData();
}






