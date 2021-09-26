#include "stdafx.h"

#include "CRuleSetDialog.hpp"
#include "CRuleSet.hpp"
#include "SOCKS5.hpp"

CRuleSetDialog::CRuleSetDialog(QWidget *parent) : QDialog(parent)
{
	this->m_addressTypeCombo = new QComboBox(this);
	this->m_methodCombo = new QComboBox(this);
	this->m_addressEdit = new QLineEdit(this);
	this->m_usernameEdit = new QLineEdit(this);
	this->m_passwordEdit = new QLineEdit(this);

	// Address Type
	this->m_addressTypeCombo->addItem("Any", ADDR_TYPE_ANY);
	this->m_addressTypeCombo->addItem("IPv4", ADDR_TYPE_IPv4);
	this->m_addressTypeCombo->addItem("IPv6", ADDR_TYPE_IPv6);

	// Methods
	this->m_methodCombo->addItem("No Authentication", SOCKS5_NO_AUTHENTICATION_REQUIRED);
	// this->m_methodCombo->addItem("GSSAPI", SOCKS5_GSSAPI);
	this->m_methodCombo->addItem("Username/Password", SOCKS5_AUTHENTICATION_REQUIRED);

	// Buttons
	QPushButton *okBtn = new QPushButton(tr("OK"), this);
	QPushButton *cancelBtn = new QPushButton(tr("Cancel"), this);

	QHBoxLayout *btnLayout = new QHBoxLayout;
	btnLayout->addStretch();
	btnLayout->addWidget(okBtn);
	btnLayout->addWidget(cancelBtn);

	QFormLayout *formLayout = new QFormLayout;
	formLayout->addRow("Type:", this->m_addressTypeCombo);
	formLayout->addRow("Address:", this->m_addressEdit);
	formLayout->addRow("Method;", this->m_methodCombo);
	formLayout->addRow("Username", this->m_usernameEdit);
	formLayout->addRow("Password", this->m_passwordEdit);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(formLayout);
	mainLayout->addLayout(btnLayout);
	this->setLayout(mainLayout);

	QObject::connect(this->m_addressTypeCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index)
	{
		this->m_addressEdit->setEnabled(index != 0);
	});
	QObject::connect(this->m_methodCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index)
	{
		this->m_usernameEdit->setEnabled(index != 0);
		this->m_passwordEdit->setEnabled(index != 0);
	});
	QObject::connect(okBtn, &QPushButton::clicked, this, &CRuleSetDialog::onOK);
	QObject::connect(cancelBtn, &QPushButton::clicked, this, &CRuleSetDialog::onCancel);

	// Set default variables
	this->m_addressTypeCombo->setCurrentIndex(0);
	this->m_methodCombo->setCurrentIndex(0);
	this->m_addressEdit->setEnabled(false);
	this->m_usernameEdit->setEnabled(false);
	this->m_passwordEdit->setEnabled(false);

	// Bluh
	this->m_accepted = false;
}

CRuleSetDialog::CRuleSetDialog(QSharedPointer<CRuleSet> &ruleSet, QWidget *parent) : CRuleSetDialog(parent)
{
	this->m_addressTypeCombo->setCurrentIndex(this->m_addressTypeCombo->findData(ruleSet->getAddrType()));
	this->m_methodCombo->setCurrentIndex(this->m_methodCombo->findData(ruleSet->getMethod()));
	this->m_addressEdit->setText(ruleSet->getHostAddress().toString());
	this->m_usernameEdit->setText(ruleSet->getUsername());
	this->m_passwordEdit->setText(ruleSet->getPassword());
}

CRuleSetDialog::~CRuleSetDialog()
{
}

QSharedPointer<CRuleSet> CRuleSetDialog::getRuleSet() const
{
	return this->m_ruleSet;
}

void CRuleSetDialog::onOK()
{
	uint8_t addrType = this->m_addressTypeCombo->currentData().toUInt();
	const QString address = this->m_addressEdit->text();
	uint8_t method = this->m_methodCombo->currentData().toUInt();
	const QString username = this->m_usernameEdit->text();
	const QString password = this->m_passwordEdit->text();

	const QHostAddress hostAddr(address);
	if (addrType == ADDR_TYPE_IPv4 && hostAddr.protocol() != QAbstractSocket::IPv4Protocol
		|| addrType == ADDR_TYPE_IPv6 && hostAddr.protocol() != QAbstractSocket::IPv6Protocol)
	{
		QMessageBox::warning(this, QString(), tr("The address you entered is invalid."));
		return;
	}

	this->m_accepted = true;
	this->m_ruleSet = QSharedPointer<CRuleSet>::create(addrType, method, hostAddr, username, password);
	this->done(QDialog::Accepted);
}

void CRuleSetDialog::onCancel()
{
	this->done(QDialog::Rejected);
}
