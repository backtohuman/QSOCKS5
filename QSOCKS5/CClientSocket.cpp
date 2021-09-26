#include "stdafx.h"

#include "CClientSocket.hpp"
#include "CMainWindow.hpp"
#include "SOCKS5.hpp"
#include "Vars.hpp"

CClientSocket::CClientSocket(QTcpSocket *socket, QObject *parent) : QObject(parent)
{
	this->m_socket = socket;
	this->m_destSocket = new QTcpSocket;
	this->m_authMethodEvaluated = false;
	this->m_authDone = false;
	this->m_commandDone = false;
	this->m_command = 0;
	this->m_addressType = 0;
	this->m_incoming = 0;
	this->m_outgoing = 0;

	QObject::connect(socket, &QTcpSocket::readyRead, this, &CClientSocket::onReadyRead);
	QObject::connect(this->m_destSocket, &QTcpSocket::connected, this, &CClientSocket::onConnected);
	QObject::connect(this->m_destSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &CClientSocket::onError);
	QObject::connect(this->m_destSocket, &QTcpSocket::readyRead, this, &CClientSocket::readFromDest);
	QObject::connect(this->m_destSocket, &QTcpSocket::disconnected, this, &CClientSocket::onDisconnected);
}

CClientSocket::~CClientSocket()
{
	this->m_socket->disconnect();
	this->m_socket->abort();

	this->m_destSocket->disconnect();
	this->m_destSocket->abort();
	this->m_destSocket->deleteLater();
}

QString CClientSocket::getIP() const
{
	return this->m_socket->peerAddress().toString();
}

uint32_t CClientSocket::getCommand() const
{
	return this->m_command;
}

QString CClientSocket::getDestinationHost() const
{
	return this->m_destSocket->peerName();
}

uint16_t CClientSocket::getDestinationPort() const
{
	return this->m_dstPort;
}

uint64_t CClientSocket::getInPackets() const
{
	return this->m_incoming;
}

uint64_t CClientSocket::getOutPackets() const
{
	return this->m_outgoing;
}

QString CClientSocket::getLastStatus() const
{
	return QString();
}

void CClientSocket::failure(uint8_t reply)
{
	qDebug() << "failure" << reply;

	uint8_t ver = 0x05;
	uint8_t rep = reply;

	// Fields marked RESERVED (RSV) must be set to X'00'.
	uint8_t rsv = 0;

	uint8_t atyp = SOCKS5_ADDR_TYPE_IPV4;
	uint32_t bnd_addr = 0;
	uint16_t bnd_port = 0;

	QByteArray response;
	QDataStream stream(&response, QIODevice::WriteOnly);
	stream << ver;
	stream << rep;
	stream << rsv;
	stream << atyp;
	stream << bnd_addr;
	stream << bnd_port;
	this->m_socket->write(response);

	// When a reply (REP value other than X'00') indicates a failure,
	// the SOCKS server MUST terminate the TCP connection shortly after sending the reply.
	// This must be no more than 10 seconds after detecting the condition that caused a failure.
	this->m_socket->disconnectFromHost();

	// lol.
	this->m_commandDone = true;
}

void CClientSocket::onAuthenticationMethod()
{
	uint8_t ver, nmethods, method, i;

	QDataStream stream(&this->m_recvedBuf, QIODevice::ReadOnly);
	if (this->m_recvedBuf.size() < 3) // ver/nmethods/method1
	{
		// Need more data
		return;
	}

	stream >> ver;
	if (ver != 0x05)
	{
		// Disconnect if it's not socks5 request
		this->m_socket->disconnectFromHost();
		return;
	}

	stream >> nmethods;
	if (this->m_recvedBuf.size() < (2 + nmethods))
	{
		// Need more data
		return;
	}

	bool foundMethod = false;
	for (uint8_t i = 0; i < nmethods; i++)
	{
		stream >> method;

		qDebug() << "authentication method:" << method;

		if (g_mainWindow->containsMethod(this->m_socket->peerAddress(), method))
		{
			qDebug() << "authentication method:" << method;
			foundMethod = true;
			break;
		}
	}

	if (!foundMethod)
	{
		qDebug() << "authentication method not found";
		method = SOCKS5_NO_ACCEPTABLE_METHODS;
	}

	QByteArray response;
	QDataStream writer(&response, QIODevice::WriteOnly);
	writer << ver;
	writer << method;
	this->m_socket->write(response);
	
	this->m_authMethodEvaluated = true;
	if (method == SOCKS5_NO_AUTHENTICATION_REQUIRED)
		this->m_authDone = true;

	this->m_recvedBuf.remove(0, 2 + nmethods);
}

void CClientSocket::onUsernamePasswordAuth()
{
	QDataStream stream(&this->m_recvedBuf, QIODevice::ReadOnly);
	if (this->m_recvedBuf.size() < 3) // ver/ulen/plen
	{
		// Need more data
		return;
	}

	uint8_t ver, ulen, plen;
	char username[256], password[256];

	stream >> ver;
	if (ver != 0x01)
	{
		this->m_socket->disconnectFromHost();
		return;
	}

	stream >> ulen;
	if (this->m_recvedBuf.size() < (3 + ulen))
	{
		// Need more data
		return;
	}
	stream.readRawData(username, ulen);
	username[ulen] = 0;

	stream >> plen;
	if (this->m_recvedBuf.size() < (3 + ulen + plen))
	{
		// Need more data
		return;
	}
	stream.readRawData(password, plen);
	password[plen] = 0;

	// The VER field contains the current version of the subnegotiation, which is X'01'.
	uint8_t rep = 0xFF;
	QByteArray response;
	QDataStream writer(&response, QIODevice::WriteOnly);
	writer << ver;
	if (g_mainWindow->authenticate(this->m_socket->peerAddress(), username, password))
	{
		// A STATUS field of X'00' indicates success.
		rep = 0x00;
		this->m_authDone = true;
	}
	writer << rep;
	this->m_socket->write(response);

	if (rep != 0x00)
	{
		// If the server returns a `failure' (STATUS value other than X'00') status, it MUST close the connection.
		this->m_socket->disconnectFromHost();
	}

	// Remove proceeded data
	this->m_recvedBuf.remove(0, 3 + ulen + plen);
}

void CClientSocket::onCommand()
{
	int read = 0;
	QDataStream stream(&this->m_recvedBuf, QIODevice::ReadOnly);
	if (this->m_recvedBuf.size() < 4) // ver/cmd/rsv/atyp
	{
		// Need more data
		return;
	}

	uint8_t ver, cmd, rsv, atyp;
	stream >> ver;
	if (ver != 0x05)
	{
		// Disconnect if it's not socks5 request
		this->m_socket->disconnectFromHost();
		return;
	}

	stream >> cmd;
	stream >> rsv;
	stream >> atyp;

	this->m_command = cmd;
	this->m_addressType = atyp;

	switch (atyp)
	{
		case SOCKS5_ADDR_TYPE_IPV4:
		{
			if (this->m_recvedBuf.size() < 10)
			{
				// Need more data
				return;
			}

			uint32_t dst_addr;
			uint16_t dst_port;

			stream >> dst_addr;
			stream >> dst_port;

			this->m_dstHost = QHostAddress(dst_addr);
			this->m_dstPort = dst_port;
			read = 10;
			break;
		}
		case SOCKS5_ADDR_TYPE_DOMAIN_NAME:
		{
			uint8_t length;
			uint16_t dst_port;

			stream >> length;

			char domainName[256];
			stream.readRawData(domainName, length);
			domainName[length] = 0;

			stream >> dst_port;
			this->m_dstPort = dst_port;

			QHostInfo::lookupHost(domainName, this, SLOT(onLookupHost(QHostInfo)));
			this->m_recvedBuf.remove(0, 4 + 1 + 2 + length);

			// Do not continue
			return;
		}
		case SOCKS5_ADDR_TYPE_IPV6:
		{
			this->failure(SOCKS5_REPLY_ADDRESS_TYPE_NOT_SUPPORTED);
			break;
		}
		default:
		{
			this->failure(SOCKS5_REPLY_ADDRESS_TYPE_NOT_SUPPORTED);
			break;
		}
	}

	switch (cmd)
	{
		case SOCKS5_CMD_CONNECT:
		{
			this->m_destSocket->connectToHost(this->m_dstHost, this->m_dstPort);
			break;
		}
		case SOCKS5_CMD_BIND:
		{
			this->failure(SOCKS5_REPLY_COMMAND_NOT_SUPPORTED);
			break;
		}
		case SOCKS5_CMD_UDP_ASSOCIATE:
		{
			this->failure(SOCKS5_REPLY_COMMAND_NOT_SUPPORTED);
			break;
		}
		default:
		{
			this->failure(SOCKS5_REPLY_COMMAND_NOT_SUPPORTED);
			break;
		}
	}

	this->m_recvedBuf.remove(0, read);

	qDebug() << "cmd" << cmd << rsv << atyp;
}

void CClientSocket::doWork()
{
	switch (this->m_command)
	{
		case SOCKS5_CMD_CONNECT:
		{
			qint64 sent = this->m_destSocket->write(this->m_recvedBuf);
			this->m_recvedBuf.remove(0, sent);
			this->m_outgoing += sent;
			break;
		}
		case SOCKS5_CMD_BIND:
		case SOCKS5_CMD_UDP_ASSOCIATE:
		default:
			break;
	}
}

void CClientSocket::processData()
{
	if (!this->m_authMethodEvaluated)
	{
		this->onAuthenticationMethod();
	}
	else if (!this->m_authDone)
	{
		this->onUsernamePasswordAuth();
	}
	else if (!this->m_commandDone)
	{
		this->onCommand();
	}
	else
	{
		this->doWork();
	}
}

void CClientSocket::onReadyRead()
{
	// Append incoming data
	const QByteArray data = this->m_socket->readAll();
	this->m_recvedBuf.append(data);

	this->processData();
}

void CClientSocket::onConnected()
{
	uint8_t ver = 0x05;
	uint8_t rep = SOCKS5_REPLY_SUCCEEDED;

	// Fields marked RESERVED (RSV) must be set to X'00'.
	uint8_t rsv = 0;

	uint8_t atyp = SOCKS5_ADDR_TYPE_IPV4;

	// Always IPv4 ATM
	uint32_t bnd_addr = this->m_destSocket->localAddress().toIPv4Address();
	uint16_t bnd_port = qFromLittleEndian<uint16_t>(this->m_destSocket->localPort());

	QByteArray response;
	QDataStream stream(&response, QIODevice::WriteOnly);
	stream << ver;
	stream << rep;
	stream << rsv;
	stream << atyp;
	stream << bnd_addr;
	stream << bnd_port;
	this->m_socket->write(response);
	this->m_commandDone = true;
}

void CClientSocket::onError(QAbstractSocket::SocketError socketError)
{
	qDebug() << "onError" << socketError;

	if (this->m_commandDone)
		return;

	uint8_t rep;
	switch (socketError)
	{
		case QAbstractSocket::ConnectionRefusedError:
			rep = SOCKS5_REPLY_CONNECTION_REFUSED;
			break;
		case QAbstractSocket::RemoteHostClosedError:
			rep = SOCKS5_REPLY_CONNECTION_REFUSED;
			break;
		case QAbstractSocket::HostNotFoundError:
			rep = SOCKS5_REPLY_HOST_UNREACHABLE;
			break;
		case QAbstractSocket::SocketTimeoutError:
			rep = SOCKS5_REPLY_TTL_EXPIRED;
			break;
		case QAbstractSocket::NetworkError:
			rep = SOCKS5_REPLY_NETWORK_UNREACHABLE;
			break;
		default:
			rep = SOCKS5_REPLY_GENERAL_FAILURE;
			break;
	}

	this->failure(rep);
}

void CClientSocket::readFromDest()
{
	const QByteArray data = this->m_destSocket->readAll();
	this->m_socket->write(data);
	this->m_incoming += data.size();
	emit this->updated();
}

void CClientSocket::onDisconnected()
{
	if (this->m_commandDone)
		return;

	this->failure(SOCKS5_REPLY_GENERAL_FAILURE);
}

void CClientSocket::onLookupHost(const QHostInfo &host)
{
	if (host.error() != QHostInfo::NoError)
	{
		this->failure(SOCKS5_REPLY_HOST_UNREACHABLE);
		return;
	}

	const auto addresses = host.addresses();
	if (addresses.empty())
	{
		this->failure(SOCKS5_REPLY_HOST_UNREACHABLE);
		return;
	}

	this->m_dstHost = addresses[0];
	switch (this->m_command)
	{
		case SOCKS5_CMD_CONNECT:
		{
			this->m_destSocket->connectToHost(this->m_dstHost, this->m_dstPort);
			break;
		}
		case SOCKS5_CMD_BIND:
		{
			this->failure(SOCKS5_REPLY_COMMAND_NOT_SUPPORTED);
			break;
		}
		case SOCKS5_CMD_UDP_ASSOCIATE:
		{
			this->failure(SOCKS5_REPLY_COMMAND_NOT_SUPPORTED);
			break;
		}
		default:
		{
			this->failure(SOCKS5_REPLY_COMMAND_NOT_SUPPORTED);
			break;
		}
	}
}
