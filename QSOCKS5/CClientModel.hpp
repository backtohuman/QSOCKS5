#pragma once

class CClientSocket;

class CClientModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit CClientModel(QObject *parent = 0);
	~CClientModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

	void appendClient(QSharedPointer<CClientSocket> &client);
	void removeClient(QSharedPointer<CClientSocket> &client);
	void updateClient(CClientSocket *client);

private:
	QList<QSharedPointer<CClientSocket>> m_items;
};