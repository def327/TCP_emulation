#include "client.h"
#include "server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyServer *server = new MyServer(9555);
    MyClient *client = new MyClient("localhost",9555);

    QWidget w;
    w.setWindowTitle("TCP Client&Server File Protocol");
    QHBoxLayout *lay = new QHBoxLayout;
    lay->addWidget(client);
    lay->addWidget(server);

    w.setLayout(lay);
    w.resize(800,600);
    w.show();
    return a.exec();
}
