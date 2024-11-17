#ifndef CSERIALPORTMANAGER_H
#define CSERIALPORTMANAGER_H
#include <QObject>
#include <deque>
#include <queue>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
class QSerialPort;
class mutex;
class memory;
class asio;
class thread;
class QByteArray;
class QString;
class io_context;
class CSerialPortManager : public QObject
{
    Q_OBJECT
public:
    explicit CSerialPortManager(QObject *parent = nullptr);
    ~CSerialPortManager();

    bool openPort(const QString &portName, int baudRate, int dataBits, int parity, int stopBits);
    void closePort();
    bool isOpen() const;

    void sendData(const QByteArray &data);
    void readData();

    void handleRead(const boost::system::error_code &error, size_t bytesTransferred);
    void handleWrite(const boost::system::error_code &error, size_t bytesTransferred);

signals:
    void signal_DataReceived(const QByteArray &data);
    void signal_ErrorOccurred(const QString &errorString);
    void signal_PortClosed();



private:
    boost::asio::io_context m_IoContext;
    std::unique_ptr<boost::asio::serial_port> m_p_SerialPort;
    // 增加一个std::future类型的成员变量来管理poll线程
    std::future<void> m_AsyncPollThread;
    std::vector<char> m_ReadBuffer;
    std::deque<boost::asio::const_buffer> m_WriteBuffer;
    std::atomic<bool> m_IsPortOpen;
    std::queue<QByteArray> m_ReceivedDataQueue;  // 接收数据队列


};

#endif // CSERIALPORTMANAGER_H
