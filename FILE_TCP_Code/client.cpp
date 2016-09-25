#include "client.h"

MyClient::MyClient(const QString &strHost,int port, QWidget *parent)
    : QWidget(parent),next_block_size(0),current_byte(0),flag_receiving(false)
{
    socket = new QTcpSocket(this);

    socket->connectToHost(strHost,port);
    connect(socket,SIGNAL(connected()),this,SLOT(slotConnected()));

    connect(socket,SIGNAL(readyRead()),this,SLOT(slotReadyRead()));

    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),
            this,SLOT(slotError(QAbstractSocket::SocketError))
            );
    txt = new QTextEdit;
    txt->setReadOnly(true);


    lbl = new QLabel;
    lbl->setText("<H2>A place for image, the client will get </H2>");
    lbl_connect = new QLabel;
    file_container.clear();

    ///Layouts-------------------------------------
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel("<H1>Client<H1>"));
    layout->addWidget(lbl_connect);
    layout->addWidget(txt);
    layout->addWidget(lbl);
    this->setLayout(layout);
}

void MyClient::slotReadyRead()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_5);
    for(;;)
    {
        if(!next_block_size)
        {
            if(socket->bytesAvailable()<sizeof(quint64))
            {
                break;
            }
            in>>next_block_size;
        }
        if( socket->bytesAvailable() < next_block_size)
        {
            break;
        }
        QByteArray tmp_array;
        in>>tmp_array;

        if(flag_receiving == false)
        {
            ///Узнали что передаётся большой файл с  величиной:
            txt->clear();
            QDataStream ds(&tmp_array,QIODevice::ReadOnly);
            ds.setVersion(QDataStream::Qt_5_5);
            ds >> MAX_FILE_SIZE;
            txt->append("Server want to give that client the PNG "
                        "image with\n" + QString::number(MAX_FILE_SIZE)
                        +" bytes");
            ///Пересылаем с какого байта переслать пакет серверу
            QByteArray current_byte_buff;
            QDataStream toserver(&current_byte_buff,QIODevice::WriteOnly);
            toserver.setVersion(QDataStream::Qt_5_5);
            toserver<<current_byte;

            ///Запрос на первый пакет
            slotSendToServer(current_byte_buff);            
            ///-----------------------------------------------
        }
        if(flag_receiving == true)
            {
                ///Добавление пакетов в обшшщий контейнер
                file_container.append(tmp_array);
                txt->append("Server give " + QString::number(file_container.size()));
                current_byte = file_container.size();

                ///В случае когда текущий байт будет больше чем
                /// максимальный размер файла выводим что в буфере на въюшку
                if(current_byte >= MAX_FILE_SIZE)
                {
                    txt->append("\n TOTAL SIZE: " + QString::number(file_container.size()));
                    QPixmap p;
                    // fill array with image
                    p.loadFromData(file_container,"PNG");
                    lbl->setPixmap(p);
                    lbl->show();
                    ///---Новые изменения
                    MAX_FILE_SIZE = 0;
                    file_container.clear();
                    current_byte = 0;
                    ///--------------------
                    flag_receiving = false;
                    next_block_size=0;
                    return;
                }

                QByteArray current_byte_buff;
                QDataStream toserver(&current_byte_buff,QIODevice::WriteOnly);
                toserver<<current_byte;

                ///Высылаем следующий байт
                slotSendToServer(current_byte_buff);

            }
        ///Установка флага
        flag_receiving = true;
        next_block_size = 0;
        break;
    }
}

void MyClient::slotError(QAbstractSocket::SocketError error)
{
    QString str_Error =
            "Error: " + (error == QAbstractSocket::HostNotFoundError ?
                         "The host was not found." :
                         error == QAbstractSocket::RemoteHostClosedError ?
                         "The remote host is closed." :
                         error == QAbstractSocket::ConnectionRefusedError ?
                         "The connection was refused." :
                         QString (socket->errorString())
                         );
    txt->append(str_Error);
}

void MyClient::slotSendToServer(const QByteArray &data)
{
    QByteArray buff;
    QDataStream out(&buff,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);
    out<<quint64(0)<<data;

    out.device()->seek(0);
    out<<quint64(buff.size()-sizeof(quint64));
    socket->write(buff);
}

void MyClient::slotConnected()
{
    lbl_connect->setText("<H2>Connect to localhost server,port 9555: OK!<H2>");
}


