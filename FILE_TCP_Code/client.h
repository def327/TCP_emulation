#ifndef CLIENT
#define CLIENT

#include <QTWidgets>
#include <QtNetwork>

class MyClient : public QWidget
{
    Q_OBJECT
    QTcpSocket *socket;
    QTextEdit *txt;
    QLabel *lbl;
    QLabel *lbl_connect;

    QByteArray file_container;
    quint64 next_block_size;
    quint64 MAX_FILE_SIZE;
    quint64 current_byte;
    bool flag_receiving;

public:
    explicit MyClient(const QString &strHost, int port, QWidget *parent = 0);

public slots:
    void slotReadyRead();
    void slotError(QAbstractSocket::SocketError);
    void slotSendToServer(const QByteArray &data);
    void slotConnected();


};


#endif // CLIENT

