#include "server.h"

MyServer::MyServer(int port, QWidget *parent)
    : QWidget(parent),next_block_size(0)
{
   server = new QTcpServer(this);
   if(!server->listen(QHostAddress::Any,port))
   {
      QMessageBox::critical(0,
                            "Server Error",
                            "Unable to start the server:"
                            + server->errorString()
                            );
      server->close();
      return;
   }

   connect(server,SIGNAL(newConnection()),
           this,SLOT(slotNewConnection()));

   txt = new QTextEdit;
   txt->setReadOnly(true);
   start_receiving = false;
   packet_size = 512;

   btn_file = new QPushButton("Choose PNG image to send");
   btn_file->setFont(QFont("TimesNewRoman",12,QFont::Bold));
   connect(btn_file,SIGNAL(clicked(bool)),this,SLOT(slotSendBigFile()));

   ///Переключатели для размера пакетов
   QFont font("TimesNewRoman",12,QFont::Bold);
   group_box = new QGroupBox();
   group_box->setTitle("The size of one packet: ");
   group_box->setFont(font);

   btn_size1 = new QRadioButton("512 bytes");
   btn_size1->setFont(font);

   btn_size2 = new QRadioButton("1024 bytes");
   btn_size2->setFont(font);

   btn_size3 = new QRadioButton("10240 bytes");
   btn_size2->setFont(font);

   btn_size1->setChecked(true);
   connect(btn_size1,SIGNAL(clicked(bool)),SLOT(slotRadioBtn1()));
   connect(btn_size2,SIGNAL(clicked(bool)),SLOT(slotRadioBtn2()));
   connect(btn_size3,SIGNAL(clicked(bool)),SLOT(slotRadioBtn3()));

   ///Layots for buttons--------------------------
   QVBoxLayout *v_lay = new QVBoxLayout;
   v_lay->addWidget(btn_size1);
   v_lay->addWidget(btn_size2);
   v_lay->addWidget(btn_size3);
   group_box->setLayout(v_lay);
   ///-------------------------------------


   ///Layouts for other elements-----------------
   QVBoxLayout *lay = new QVBoxLayout;
   lay->addWidget(new QLabel("<H1>Server<H1>"));
   lay->addWidget(txt);
   lay->addWidget(group_box);
   lay->addWidget(btn_file);
   this->setLayout(lay);
}

void MyServer::slotNewConnection()
{
    QTcpSocket *client_socket = server->nextPendingConnection();

    copy_socket = client_socket;  //сохраняем настройки сокета в объект класса сервера

    connect(client_socket,SIGNAL(disconnected()),
            client_socket,SLOT(deleteLater()));

    connect(client_socket,SIGNAL(readyRead()),
            this,SLOT(slotReadClient()));
}

void MyServer::slotReadClient()
{
    QTcpSocket *client_socket = (QTcpSocket*)sender();
    copy_socket = client_socket;
    QDataStream in(client_socket);
    in.setVersion(QDataStream::Qt_5_5);
    for(;;)
    {
        if(!next_block_size)
        {
            if(client_socket->bytesAvailable() < sizeof(quint64))
            {
                break;
            }
            in>>next_block_size;
        }
        if(client_socket->bytesAvailable() < next_block_size)
        {
            break;
        }
        ///Приём с какого байта выслать пакет
        QByteArray tmp_position;
        in>>tmp_position;

        quint64 position;
        QDataStream read_pos(&tmp_position,QIODevice::ReadOnly);
        read_pos.setVersion(QDataStream::Qt_5_5);
        read_pos >> position;
        txt->append("Client want the bytes starting at " + QString::number(position));

        ///Создание пакета для отправки клиенту
        ///Картинка с жесткого диска
        QFile file(file_name_str);
        if(!file.open(QIODevice::ReadOnly))
        {
            qDebug()<<"Problem with the path to image!";
            return;
        }
        QByteArray client_pack;
        QDataStream out(&client_pack, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_5);
        file.seek(position);
        out<< file.read(packet_size);
        file.close();

        ///QDataStream дополнительно еще записывает 4 байта(для размера)
        ///Делаем пакет без этих 4 байтов для длины буфера
        QDataStream finish(&client_pack,QIODevice::ReadOnly);
        finish.setVersion(QDataStream::Qt_5_5);
        finish.device()->seek(sizeof(quint32));

        QByteArray send_packet;
        send_packet.append(finish.device()->read(client_pack.size()));

        ///--------------исправления
        txt->append("Sent to server from " + QString::number(position)
                    + " to " + QString::number(position +send_packet.size()));

        ///Перекидываем сделанный пакет клиенту
        this->sendToClient(copy_socket,send_packet);

        ///----------------------------------------------------------
        next_block_size=0;
        break;
    }

}

void MyServer::sendToClient(QTcpSocket *Socket, const QByteArray  &data)
{
    QByteArray buff_socket;
    QDataStream out(&buff_socket,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);
    out<<quint64(0)<<data;

        //Для записи размера файла!!!

        out.device()->seek(0);
        out<<quint64(buff_socket.size()- sizeof(quint64));
        Socket->write(buff_socket);
        txt->append("Server give the file to client");
}

void MyServer::slotSendBigFile()
{
    ///Считываем имя картинки в file_name_str
    file_name_str = QFileDialog::getOpenFileName(0,"Select PNG image","","*.png");
    QFile file(file_name_str);
    if(file_name_str == "" | !file.open(QIODevice::ReadOnly))
     {
        file.close();
        return;
     }

    ///---Для отладки
    /*
    file_name_str = "Picture.png"; //---картинка находится в корневой папке с программой
    QFile file(file_name_str);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"Problem with the path to image!";
        return;
    }
    */
    ///--------------

    ///Максимальный размер файла
    maxSizeFile=file.size();
    file.close();

    ///Начать поэтапную передачу между сервером и клиентом
    start_receiving = true;

    ///Закидываем максимальный размер файла в массив байтов и оправялем его клиенту
    QByteArray buff_size;
    QDataStream out(&buff_size,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);
    out<<maxSizeFile;
    this->sendToClient(copy_socket,buff_size);
}

///---Переключатели размеров для пакета
void MyServer::slotRadioBtn1()
{
   packet_size = 512;
}

void MyServer::slotRadioBtn2()
{
   packet_size = 1024;
}

void MyServer::slotRadioBtn3()
{
   packet_size = 10240;
}
