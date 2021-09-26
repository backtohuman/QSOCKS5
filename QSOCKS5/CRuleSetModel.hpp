#pragma once

class CRuleSet;

class CRuleSetModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	explicit CRuleSetModel(QObject *parent = 0);
	~CRuleSetModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

	void append(const QSharedPointer<CRuleSet> &ruleSet);
	QSharedPointer<CRuleSet> get(int row) const;
	void set(int row, QSharedPointer<CRuleSet> &ruleSet);
	void remove(int row);

private:
	QList<QSharedPointer<CRuleSet>> m_items;
};