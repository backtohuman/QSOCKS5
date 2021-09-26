#pragma once

enum
{
	ADDR_TYPE_ANY = 0,
	ADDR_TYPE_IPv4,
	ADDR_TYPE_IPv6
};

class CRuleSet
{
public:
	CRuleSet(uint8_t addrType = 0, uint8_t method = 0, const QHostAddress &hostAddr = QHostAddress(),
		const QString &username = QString(), const QString password = QString());
	~CRuleSet();

	uint8_t getAddrType() const;
	uint8_t getMethod() const;
	QHostAddress getHostAddress() const;
	QString getUsername() const;
	QString getPassword() const;

	QByteArray saveState(int version = 0) const;
	bool restoreState(const QByteArray &state, int version = 0);

	bool authenticate(const QHostAddress &hostAddress) const;
	bool authenticate(const QHostAddress &hostAddress, const QString &username, const QString &password) const;

private:
	uint8_t m_addrType, m_method;
	QHostAddress m_hostAddr;
	QString m_username, m_password;
};