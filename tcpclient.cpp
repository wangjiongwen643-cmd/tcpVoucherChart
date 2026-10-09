#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QFileDialog>
#include <QFile>
#include <QLabel>
#include <QLayoutItem>
#include <QMessageBox>
#include <QHostAddress>
#include <QDebug>
#include <QPixmap>
#include <QPushButton>

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, [=](){
        qDebug() << "连接成功";
    });
    connect(socket, &QTcpSocket::errorOccurred, this, [=](QAbstractSocket::SocketError err){
        qDebug() << "连接失败:" << socket->errorString();
    });
    ui->scrollArea->setWidgetResizable(true);
}

TcpClient::~TcpClient()
{
    delete ui;
}

//连接服务器按钮
void TcpClient::on_pushButtonConfirmConnect_clicked()
{
    QString ipStr = ui->textEditInputServiceIP->toPlainText().trimmed();
    QString portStr = ui->textEditInputPortNumber->toPlainText().trimmed();
    if(ipStr.isEmpty() || portStr.isEmpty())
    {
        QMessageBox::information(this,"提示","内容不能为空");
        return;
    }
    bool ok;
    quint16 port = portStr.toUShort(&ok);
    if(!ok)
    {
        QMessageBox::warning(this,"错误","端口不是合法数字");
        return;
    }
    if(socket->state() == QTcpSocket::ConnectingState || socket->state() == QTcpSocket::ConnectedState)
    {
        QMessageBox::information(this,"提示","已经在连接/已连接");
        return;
    }
    adding++;
    if(adding >= addingMax)
    {
        QMessageBox::warning(this,"错误!","您的请求次数过多,请关闭并重启程序");
        return;
    }
    socket->connectToHost(QHostAddress(ipStr), port);
}

//选择上传文件按钮
void TcpClient::on_InputFilesButton_clicked()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this,"选择要上传的文件","../");
    if(filePaths.isEmpty())
    {
        qDebug() << "用户取消选择文件";
        return;
    }
    QWidget* containerWidget = ui->scrollArea->widget();
    if (!containerWidget)
        return;
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(containerWidget->layout());
    if (!layout)
    {
        layout = new QVBoxLayout(containerWidget);
        layout->setAlignment(Qt::AlignTop);
        layout->setSpacing(10);
    }
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if(item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    mFileList.clear();
    for(const QString& filePath : filePaths)
    {
        mFileList.append(filePath);
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
        //删除按钮
        QPushButton* btnDelete = new QPushButton("删除");
        btnDelete->setFixedWidth(70);
        hLayout->addWidget(imgLabel);
        hLayout->addWidget(nameLabel,1);
        hLayout->addWidget(btnDelete);
        layout->addWidget(itemWidget);
        //点击删除：移除界面条目 + 删除文件列表记录
        connect(btnDelete, &QPushButton::clicked, this, [=](){
            mFileList.removeOne(filePath);
            itemWidget->deleteLater();
        });
    }
}

//撤回全部按钮
void TcpClient::on_pushButton_5_clicked()
{
    QWidget* containerWidget = ui->scrollArea->widget();
    if (!containerWidget)
        return;
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(containerWidget->layout());
    if (!layout)
        return;
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if(item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    mFileList.clear();
}

//全部发送按钮
void TcpClient::on_pushButton_6_clicked()
{
    if(socket->state() != QTcpSocket::ConnectedState)
    {
        QMessageBox::warning(this, "提示", "尚未连接服务器，请先建立连接！");
        return;
    }
    if(mFileList.isEmpty())
    {
        QMessageBox::information(this, "提示", "暂无待上传文件，请先选择文件");
        return;
    }

    for(const QString& filePath : mFileList)
    {
        QFile file(filePath);
        if(!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, "错误", "文件打开失败：" + filePath);
            continue;
        }
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName();
        qint64 fileSize = fileInfo.size();


        QString header = QString("%1|%2\n").arg(fileName).arg(fileSize);
        socket->write(header.toUtf8());

        QByteArray fileData = file.readAll();
        socket->write(fileData);
        socket->waitForBytesWritten(3000);

        file.close();
        qDebug() << "文件发送完成：" << filePath;
    }
    QMessageBox::information(this, "上传完成", "所有选中文件的发送数据已发出！");
}
