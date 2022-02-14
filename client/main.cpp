#include <QtCore/QCoreApplication>
#include <QtNetwork>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QTcpSocket *socket = new QTcpSocket;
	socket->setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, "127.0.0.1", 58934));
	socket->connectToHost("www.google.com", 80);
	if (socket->waitForConnected())
	{
		qDebug() << "Connected!";
	}
	else
	{
		qDebug() << "failed to connect with error:" << socket->errorString();
	}
	socket->disconnectFromHost();
	socket->deleteLater();

	return a.exec();
}
