#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include <QWidget>
//TCP通信
#include <QTcpSocket>
#include <QTcpServer>
//窗口以及调试
#include <QDebug>
#include <QMessageBox>
#include <QString>
//文件上传以及获取
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
QT_BEGIN_NAMESPACE
namespace Ui {
class TcpClient;
}
QT_END_NAMESPACE
class TcpClient : public QWidget
{
    Q_OBJECT
public:
    explicit TcpClient(QWidget *parent = nullptr);
    ~TcpClient() override;
private slots:
    void on_pushButtonConfirmConnect_clicked();
    void on_InputFilesButton_clicked();

    //撤回全部
    void on_pushButton_5_clicked();
    //全部发送
    void on_pushButton_6_clicked();
private:
    Ui::TcpClient *ui;
    int adding = 0;
    const int addingMax = 5;
    QTcpSocket *socket;

    //文件路径
    QStringList mFileList;
};
#endif // TCPCLIENT_H
