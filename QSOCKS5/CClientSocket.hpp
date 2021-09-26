#pragma once

class CClientSocket : public QObject
{
	Q_OBJECT

public:
	CClientSocket(QTcpSocket *socket, QObject *parent = 0);
	~CClientSocket();

	QString getIP() const;
	uint32_t getCommand() const;
	QString getDestinationHost() const;
	uint16_t getDestinationPort() const;
	uint64_t getInPackets() const;
	uint64_t getOutPackets() const;
	QString getLastStatus() const;

	void failure(uint8_t reply);
	void onAuthenticationMethod();
	void onUsernamePasswordAuth();
	void onCommand();
	void doWork();
	void processData();

private slots:
	void onReadyRead();
	void onConnected();
	void onError(QAbstractSocket::SocketError socketError);
	void readFromDest();
	void onDisconnected();
	void onLookupHost(const QHostInfo &host);

signals:
	void updated();

private:
	QTcpSocket *m_socket, *m_destSocket;
	bool m_authMethodEvaluated, m_authDone, m_commandDone;
	QByteArray m_recvedBuf, m_sendBuf;
	uint8_t m_command, m_addressType;

	QHostAddress m_dstHost;
	uint16_t m_dstPort;
	uint64_t m_incoming, m_outgoing;
};