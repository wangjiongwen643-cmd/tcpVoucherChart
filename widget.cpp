#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    QDir dir;
    dir.mkpath(saveRootPath);

    QWidget* container = ui->scrollArea->widget();
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setAlignment(Qt::AlignTop);
    layout->setSpacing(10);
    container->setLayout(layout);

    //监听端口他妈的必须和客户端保持一致
    server.listen(QHostAddress::Any,8888);
    connect(&server, &QTcpServer::newConnection, this, &Widget::onNewConnection);
    qDebug() << "服务端启动，监听端口8888";
}

Widget::~Widget()
{
    if(clientSocket)
    {
        clientSocket->disconnectFromHost();
        clientSocket->deleteLater();
    }
    delete ui;
}

void Widget::onNewConnection()
{
    clientSocket = server.nextPendingConnection();
    connect(clientSocket,&QTcpSocket::readyRead,this,&Widget::onReadyRead);
    connect(clientSocket,&QTcpSocket::disconnected,this,&Widget::onClientDisconnect);
    qDebug() << "客户端已接入";
}

void Widget::onReadyRead()
{
    while(clientSocket->bytesAvailable()>0)
    {
        if(needReadBytes <= 0)
        {

            QByteArray line = clientSocket->readLine();
            QString header = QString::fromUtf8(line).trimmed();
            QStringList infoList = header.split("|");
            if(infoList.size() < 2) continue;

            recvFileName = infoList[0];
            needReadBytes = infoList[1].toLongLong();

            QString fullSavePath = saveRootPath + recvFileName;
            recvFile.setFileName(fullSavePath);
            if(!recvFile.open(QIODevice::WriteOnly))
            {
                qDebug() << "文件创建失败:" << fullSavePath;
                needReadBytes = 0;
                return;
            }
            qDebug() << "准备接收文件：" << recvFileName << " 大小:" << needReadBytes;
        }
        else
        {
            QByteArray data = clientSocket->read(needReadBytes);
            recvFile.write(data);
            needReadBytes -= data.size();

            if(needReadBytes <= 0)
            {
                recvFile.close();
                QString fullPath = saveRootPath + recvFileName;
                qDebug() << recvFileName << "接收完成保存路径:" << fullPath;
                //收到文件后，在界面添加预览条目
                addFilePreviewItem(fullPath);
            }
        }
    }
}

void Widget::onClientDisconnect()
{
    qDebug() << "客户端断开连接";
    if(recvFile.isOpen())
        recvFile.close();
    clientSocket->deleteLater();
    clientSocket = nullptr;
    needReadBytes = 0;
}

//添加文件预览条目到scrollarea
void Widget::addFilePreviewItem(const QString &filePath)
{
    QWidget* container = ui->scrollArea->widget();
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(container->layout());
    if(!layout) return;

    QFileInfo fileInfo(filePath);
    QWidget* itemWidget = new QWidget();
    QHBoxLayout* hLayout = new QHBoxLayout(itemWidget);
    hLayout->setContentsMargins(4,4,4,4);

    QLabel* imgLabel = new QLabel();
    imgLabel->setFixedSize(80,80);
    QPixmap pixmap(filePath);
    if(!pixmap.isNull())
    {
        pixmap = pixmap.scaled(imgLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imgLabel->setPixmap(pixmap);
    }
    else
    {
        imgLabel->setText(fileInfo.fileName());
        imgLabel->setAlignment(Qt::AlignCenter);
    }
    QLabel* nameLabel = new QLabel(filePath);
    nameLabel->setWordWrap(true);

    hLayout->addWidget(imgLabel);
    hLayout->addWidget(nameLabel,1);
    layout->addWidget(itemWidget);
}

void Widget::clearPreviewList()
{
    QWidget* container = ui->scrollArea->widget();
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(container->layout());
    if(!layout) return;
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if(item->widget())
            item->widget()->deleteLater();
        delete item;
    }
}
