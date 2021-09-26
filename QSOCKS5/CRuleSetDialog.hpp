#pragma once

class CRuleSet;

class CRuleSetDialog : public QDialog
{
	Q_OBJECT

public:
	CRuleSetDialog(QWidget *parent = 0);
	CRuleSetDialog(QSharedPointer<CRuleSet> &ruleSet, QWidget *parent = 0);
	~CRuleSetDialog();

	QSharedPointer<CRuleSet> getRuleSet() const;

private slots:
	void onOK();
	void onCancel();

private:
	QComboBox *m_addressTypeCombo, *m_methodCombo;
	QLineEdit *m_addressEdit, *m_usernameEdit, *m_passwordEdit;

	bool m_accepted;
	QSharedPointer<CRuleSet> m_ruleSet;
};