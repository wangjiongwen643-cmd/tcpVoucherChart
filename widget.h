#ifndef WIDGET_H
#define WIDGET_H
#include <QWidget>
//TCP
#include <QTcpServer>
#include <QTcpSocket>
//文件操作
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnect();

private:
    Ui::Widget *ui;
    QTcpSocket* clientSocket = nullptr;
    QTcpServer server;

    //文件接收
    QString recvFileName;
    qint64 needReadBytes = 0;
    QFile recvFile;
    const QString saveRootPath = "/home/ubuntu/TcpServerInUbuntu/pics";

    // 在scrollarea添加收到的文件预览条目
    void addFilePreviewItem(const QString &filePath);
    //清空预览列表
    void clearPreviewList();
};
#endif // WIDGET_H
