#include "stdafx.h"

#include "CRuleSetModel.hpp"
#include "CRuleSet.hpp"
#include "SOCKS5.hpp"

CRuleSetModel::CRuleSetModel(QObject *parent) : QAbstractTableModel(parent)
{
}

CRuleSetModel::~CRuleSetModel()
{
}

void CRuleSetModel::append(const QSharedPointer<CRuleSet> &ruleSet)
{
	this->beginInsertRows(QModelIndex(), this->m_items.size(), this->m_items.size());

	this->m_items.push_back(ruleSet);

	this->endInsertRows();
}

QSharedPointer<CRuleSet> CRuleSetModel::get(int row) const
{
	if (row < 0 || this->rowCount() <= row)
		return QSharedPointer<CRuleSet>();
	return this->m_items[row];
}

void CRuleSetModel::set(int row, QSharedPointer<CRuleSet>& ruleSet)
{
	if (row < 0 || this->rowCount() <= row)
		return;
	this->m_items[row] = ruleSet;
	emit this->dataChanged(this->createIndex(row, 0), this->createIndex(row, this->columnCount() - 1));
}

void CRuleSetModel::remove(int row)
{
	// モデルからデータを削除する前に呼び出す。
	this->beginRemoveRows(QModelIndex(), row, row);

	// 実際のデータから削除
	this->m_items.removeAt(row);

	// モデルからデータを削除した後に呼び出す。
	this->endRemoveRows();
}

int CRuleSetModel::rowCount(const QModelIndex &parent) const
{
	return this->m_items.size();
}

int CRuleSetModel::columnCount(const QModelIndex &parent) const
{
	return 4;
}

QVariant CRuleSetModel::data(const QModelIndex &index, int role) const
{
	if (index.row() >= this->m_items.size())
	{
		// Invalid
		return QVariant();
	}

	auto item = this->m_items[index.row()];
	if (role == Qt::DisplayRole)
	{
		auto getAddrTypeString = [](uint8_t addrType) -> QString
		{
			switch (addrType)
			{
				case ADDR_TYPE_ANY: return QString("Any");
				case ADDR_TYPE_IPv4: return QString("IPv4");
				case ADDR_TYPE_IPv6: return QString("IPv6");
				default: return QString("Invalid");
			}
		};

		auto getMethodString = [](uint8_t method) -> QString
		{
			switch (method)
			{
				case SOCKS5_NO_AUTHENTICATION_REQUIRED: return QString("No Auth Required");
				case SOCKS5_GSSAPI: return QString("GSSAPI");
				case SOCKS5_AUTHENTICATION_REQUIRED: return QString("Username/Password");
				default: return QString("Invalid");
			}
		};

		switch (index.column())
		{
			case 0: return getAddrTypeString(item->getAddrType());
			case 1: return item->getHostAddress().toString();
			case 2: return getMethodString(item->getMethod());
			case 3: return item->getUsername();
		}
	}

	return QVariant();
}

QVariant CRuleSetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (section)
		{
			case 0: return QString("Type");
			case 1: return QString("Address");
			case 2: return QString("Method");
			case 3: return QString("Username");
		}
	}

	return QVariant();
}

Qt::ItemFlags CRuleSetModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = QAbstractTableModel::flags(index);
	return flags;
}