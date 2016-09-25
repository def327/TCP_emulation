#ifndef SERVER
#define SERVER

#include <QTWidgets>
#include <QtNetwork>


class MyServer : public QWidget
{
    Q_OBJECT
    QTcpServer *server;
    QTcpSocket *copy_socket; //Для хранения настроек сокета
    QTextEdit *txt;
    QPushButton *btn_file;
    QGroupBox *group_box;
    QRadioButton *btn_size1;
    QRadioButton *btn_size2;
    QRadioButton *btn_size3;

    quint64 next_block_size;
    quint64 maxSizeFile;
    bool start_receiving;
    int  packet_size;
    QString file_name_str;

    void sendToClient(QTcpSocket* Socket, const QByteArray &data);

public:
    explicit MyServer(int port, QWidget *parent = 0);

public slots:
    void slotNewConnection();
    void slotReadClient();
    void slotSendBigFile();

    void slotRadioBtn1();
    void slotRadioBtn2();
    void slotRadioBtn3();
};



#endif // SERVER

