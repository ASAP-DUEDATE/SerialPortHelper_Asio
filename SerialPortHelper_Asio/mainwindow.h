#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "cserialportmanager.h"

#include <QMainWindow>
class QPushButton;
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void init();
    void updateUIOnPortChange(QPushButton *button, bool isPortOpen);

private slots:
    void on_pushButton_OpenSendPort_clicked();
    void on_pushButton_OpenRecPort_clicked();
    void on_pushButton_Send_clicked();
    void on_pushButton_Clean_clicked();

    void handleDataReceived(const QByteArray &data);
    void handleSerialportError(const QString &error);

private:
    Ui::MainWindow *ui;
    std::unique_ptr<CSerialPortManager> m_p_SendSerialPortManager;
    std::unique_ptr<CSerialPortManager> m_p_RecSerialPortManager;
};
#endif // MAINWINDOW_H
