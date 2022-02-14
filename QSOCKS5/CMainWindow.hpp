#pragma once

class CClientModel;
class CRuleSetModel;
class CClientSocket;

class CMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CMainWindow(QWidget *parent = 0);
	~CMainWindow();

protected:
	void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

public:
	bool authenticate(const QHostAddress &hostAddr, const QString &username, const QString &password) const;
	bool containsMethod(const QHostAddress &hostAddr, uint8_t method) const;

private slots:
	void onStart();
	void onStop();
	void onRestart();

	void onRuleNew();
	void onRuleEdit();
	void onRuleDelete();

	void onClientContextMenu(const QPoint &pos);
	void onRuleSetContextMenu(const QPoint &pos);

	// server
	void onAcceptError(QAbstractSocket::SocketError socketError);
	void onNewConnection();

	// client
	void onDisconnected();
	void onError(QAbstractSocket::SocketError);
	void onClientUpdated();

public slots:
	void loadSettings();
	void saveSettings();

private:
	QList<QTcpSocket *> m_sockets;
	QMap<QTcpSocket *, QSharedPointer<CClientSocket>> m_clients;
	QTcpServer *m_tcpServer;

	QSplitter *m_splitter;
	QPushButton *m_startButton, *m_stopButton, *m_restartButton;

	CClientModel *m_clientModel;
	CRuleSetModel *m_ruleModel;
	QTableView *m_clientView, *m_ruleView;

	QAction *m_ruleNewAct, *m_ruleEditAct, *m_ruleDeleteAct;
};