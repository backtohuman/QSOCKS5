#include <QtCore/QCoreApplication>
#include <QtNetwork>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QTcpSocket* socket = new QTcpSocket;
	socket->setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, "127.0.0.1", 58934, "admin", "admin"));
	socket->connectToHost("www.google.com", 80);
	qDebug() << socket->waitForConnected();
	qDebug() << socket->errorString();

    return a.exec();
}
