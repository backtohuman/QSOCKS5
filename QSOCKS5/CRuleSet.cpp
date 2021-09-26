#include "stdafx.h"

#include "CRuleSet.hpp"
#include "SOCKS5.hpp"

CRuleSet::CRuleSet(uint8_t addrType, uint8_t method, 
	const QHostAddress &hostAddr, const QString &username, const QString password)
{
	this->m_addrType = addrType;
	this->m_method = method;
	this->m_hostAddr = hostAddr;
	this->m_username = username;
	this->m_password = password;
}

CRuleSet::~CRuleSet()
{

}

uint8_t CRuleSet::getAddrType() const
{
	return this->m_addrType;
}

uint8_t CRuleSet::getMethod() const
{
	return this->m_method;
}

QHostAddress CRuleSet::getHostAddress() const
{
	return this->m_hostAddr;
}

QString CRuleSet::getUsername() const
{
	return this->m_username;
}

QString CRuleSet::getPassword() const
{
	return this->m_password;
}

QByteArray CRuleSet::saveState(int version) const
{
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);
	stream << this->m_addrType;
	stream << this->m_method;
	stream << this->m_hostAddr;
	stream << this->m_username;
	stream << this->m_password;
	return data;
}

bool CRuleSet::restoreState(const QByteArray &state, int version)
{
	QByteArray data = state;
	QDataStream stream(&data, QIODevice::ReadOnly);
	stream >> this->m_addrType;
	stream >> this->m_method;
	stream >> this->m_hostAddr;
	stream >> this->m_username;
	stream >> this->m_password;
	return true;
}

bool CRuleSet::authenticate(const QHostAddress &hostAddress) const
{
	switch (this->m_addrType)
	{
		case ADDR_TYPE_ANY:
			return true;

		case ADDR_TYPE_IPv4:
		{
			bool ok1, ok2;
			quint32 IPv4_1 = hostAddress.toIPv4Address(&ok1);
			quint32 IPv4_2 = this->m_hostAddr.toIPv4Address(&ok2);
			return ok1 && ok2 && IPv4_1 == IPv4_2;
		}
		case ADDR_TYPE_IPv6:
		{
			Q_IPV6ADDR addr1 = hostAddress.toIPv6Address();
			Q_IPV6ADDR addr2 = this->m_hostAddr.toIPv6Address();
			return memcmp(&addr1, &addr2, sizeof(Q_IPV6ADDR)) == 0;
		}
	}
	return false;
}

bool CRuleSet::authenticate(const QHostAddress &hostAddress, const QString &username, const QString &password) const
{
	if (!this->authenticate(hostAddress))
		return false;

	if (this->m_method == SOCKS5_NO_AUTHENTICATION_REQUIRED)
		return true;

	return this->m_username == username && this->m_password == password;
}
