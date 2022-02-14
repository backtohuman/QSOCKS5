#include "stdafx.h"

#include "CClientModel.hpp"
#include "CClientSocket.hpp"

CClientModel::CClientModel(QObject *parent) : QAbstractTableModel(parent)
{
}

CClientModel::~CClientModel()
{
}

int CClientModel::rowCount(const QModelIndex &parent) const
{
	return this->m_items.size();
}

int CClientModel::columnCount(const QModelIndex &parent) const
{
	return 6;
}

QVariant CClientModel::data(const QModelIndex &index, int role) const
{
	if (index.row() >= this->m_items.size())
	{
		// Invalid
		return QVariant();
	}

	auto item = this->m_items[index.row()];
	if (role == Qt::DisplayRole)
	{
		switch (index.column())
		{
			case 0: return item->getIP();
			case 1: return item->getCommand();
			case 2: return QString("%1::%2").arg(item->getDestinationHost()).arg(item->getDestinationPort());
			case 3: return item->getInPackets();
			case 4: return item->getOutPackets();
			case 5: return item->getLastStatus();
		}
	}

	return QVariant();
}

QVariant CClientModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (section)
		{
			case 0: return QString("IP");
			case 1: return QString("Command");
			case 2: return QString("Destination");
			case 3: return QString("In packets");
			case 4: return QString("Out packets");
			case 5: return QString("Status");
		}
	}

	return QVariant();
}

Qt::ItemFlags CClientModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = QAbstractTableModel::flags(index);
	return flags;
}

void CClientModel::appendClient(QSharedPointer<CClientSocket> client)
{
	this->beginInsertRows(QModelIndex(), this->m_items.size(), this->m_items.size());

	this->m_items.push_back(client);

	this->endInsertRows();
}

void CClientModel::removeClient(QSharedPointer<CClientSocket> client)
{
	int row = this->m_items.indexOf(client);
	if (row == -1)
		return;

	// ���f������f�[�^���폜����O�ɌĂяo���B
	this->beginRemoveRows(QModelIndex(), row, row);

	// ���ۂ̃f�[�^����폜
	this->m_items.removeAt(row);

	// ���f������f�[�^���폜������ɌĂяo���B
	this->endRemoveRows();
}

void CClientModel::updateClient(CClientSocket *client)
{
	for (int row = 0; row < this->rowCount(); row++)
	{
		if (this->m_items[row].data() == client)
		{
			emit this->dataChanged(this->createIndex(row, 0), this->createIndex(row, this->columnCount() - 1));
			break;
		}
	}
}
