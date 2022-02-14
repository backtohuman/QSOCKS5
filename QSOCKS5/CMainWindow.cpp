#include "stdafx.h"

#include "CMainWindow.hpp"
#include "CClientModel.hpp"
#include "CClientSocket.hpp"
#include "CRuleSetModel.hpp"
#include "CRuleSetDialog.hpp"
#include "CRuleSet.hpp"

CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent)
{
	this->m_tcpServer = new QTcpServer;
	QObject::connect(this->m_tcpServer, &QTcpServer::acceptError, this, &CMainWindow::onAcceptError);
	QObject::connect(this->m_tcpServer, &QTcpServer::newConnection, this, &CMainWindow::onNewConnection);

	// Client View
	this->m_clientView = new QTableView(this);
	this->m_clientModel = new CClientModel(this->m_clientView);
	this->m_clientView->setModel(this->m_clientModel);
	this->m_clientView->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->m_clientView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	this->m_clientView->horizontalHeader()->setStretchLastSection(true);
	QObject::connect(this->m_clientView, &QTableView::customContextMenuRequested, this, &CMainWindow::onClientContextMenu);

	// Rule View
	this->m_ruleView = new QTableView(this);
	this->m_ruleModel = new CRuleSetModel(this->m_ruleView);
	this->m_ruleView->setModel(this->m_ruleModel);
	this->m_ruleView->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->m_ruleView->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	this->m_ruleView->horizontalHeader()->setStretchLastSection(true);
	QObject::connect(this->m_ruleView, &QTableView::customContextMenuRequested, this, &CMainWindow::onRuleSetContextMenu);

	// Bottom
	this->m_startButton = new QPushButton(tr("Start"), this);
	this->m_stopButton = new QPushButton(tr("Stop"), this);
	this->m_restartButton = new QPushButton(tr("Restart"), this);
	QObject::connect(this->m_startButton, &QPushButton::clicked, this, &CMainWindow::onStart);
	QObject::connect(this->m_stopButton, &QPushButton::clicked, this, &CMainWindow::onStop);
	QObject::connect(this->m_restartButton, &QPushButton::clicked, this, &CMainWindow::onRestart);

	this->m_splitter = new QSplitter(this);
	this->m_splitter->setOrientation(Qt::Orientation::Horizontal);
	this->m_splitter->addWidget(this->m_clientView);
	this->m_splitter->addWidget(this->m_ruleView);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(this->m_startButton);
	buttonLayout->addWidget(this->m_stopButton);
	buttonLayout->addWidget(this->m_restartButton);

	QVBoxLayout *centralWidgetLayout = new QVBoxLayout;
	centralWidgetLayout->addWidget(this->m_splitter, 1);
	centralWidgetLayout->addLayout(buttonLayout);

	QWidget *centralWidget = new QWidget(this);
	centralWidget->setLayout(centralWidgetLayout);

	this->setCentralWidget(centralWidget);
	this->setWindowTitle(tr("QSOCKS5"));
	this->statusBar()->showMessage(tr(""));

	this->m_ruleNewAct = new QAction(tr("New"), this);
	this->m_ruleEditAct = new QAction(tr("Edit"), this);
	this->m_ruleDeleteAct = new QAction(tr("Delete"), this);
	QObject::connect(this->m_ruleNewAct, &QAction::triggered, this, &CMainWindow::onRuleNew);
	QObject::connect(this->m_ruleEditAct, &QAction::triggered, this, &CMainWindow::onRuleEdit);
	QObject::connect(this->m_ruleDeleteAct, &QAction::triggered, this, &CMainWindow::onRuleDelete);

	this->loadSettings();
}
CMainWindow::~CMainWindow()
{
	this->m_tcpServer->disconnect();
	this->m_tcpServer->close();
	this->m_tcpServer->deleteLater();
}

void CMainWindow::closeEvent(QCloseEvent *event)
{
	QMessageBox msgBox;
	msgBox.setText(tr("Are you sure you want to quit?"));
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Cancel);
	if (msgBox.exec() == QMessageBox::Cancel)
	{
		// do not close
		event->ignore();
	}
	else
	{
		this->saveSettings();
		event->accept();
	}
}

bool CMainWindow::authenticate(const QHostAddress &hostAddr, const QString &username, const QString &password) const
{
	for (int row = 0; row < this->m_ruleModel->rowCount(); row++)
	{
		const auto ruleSet = this->m_ruleModel->get(row);
		if (ruleSet->authenticate(hostAddr, username, password))
			return true;
	}
	return false;
}

bool CMainWindow::containsMethod(const QHostAddress &hostAddr, uint8_t method) const
{
	for (int row = 0; row < this->m_ruleModel->rowCount(); row++)
	{
		const auto ruleSet = this->m_ruleModel->get(row);
		if (ruleSet->authenticate(hostAddr) && ruleSet->getMethod() == method)
			return true;
	}
	return false;
}

void CMainWindow::onStart()
{
	if (this->m_tcpServer->listen(QHostAddress::Any, 58934))
	{
		this->m_startButton->setEnabled(false);
		this->m_stopButton->setEnabled(true);
		this->m_restartButton->setEnabled(true);
	}
}

void CMainWindow::onStop()
{
	this->m_tcpServer->close();
	foreach (auto socket, this->m_sockets)
		socket->disconnectFromHost();

	this->m_startButton->setEnabled(true);
	this->m_stopButton->setEnabled(false);
	this->m_restartButton->setEnabled(false);
}

void CMainWindow::onRestart()
{
	this->onStop();
	this->onStart();
}

void CMainWindow::onRuleNew()
{
	CRuleSetDialog dialog(this);
	if (dialog.exec() == QDialog::Rejected)
		return;

	auto ruleSet = dialog.getRuleSet();
	this->m_ruleModel->append(ruleSet);
}

void CMainWindow::onRuleEdit()
{
	const QModelIndex index = this->m_ruleView->currentIndex();
	if (!index.isValid())
		return;

	auto ruleSet = this->m_ruleModel->get(index.row());

	CRuleSetDialog dialog(ruleSet, this);
	if (dialog.exec() == QDialog::Rejected)
		return;

	auto ruleSet2 = dialog.getRuleSet();
	this->m_ruleModel->set(index.row(), ruleSet2);
}

void CMainWindow::onRuleDelete()
{
	const QModelIndex index = this->m_ruleView->currentIndex();
	if (!index.isValid())
		return;

	this->m_ruleModel->remove(index.row());
}

void CMainWindow::onClientContextMenu(const QPoint &pos)
{
}

void CMainWindow::onRuleSetContextMenu(const QPoint &pos)
{
	const QModelIndex index = this->m_ruleView->indexAt(pos);
	bool valid = index.isValid();

	this->m_ruleNewAct->setEnabled(true);
	this->m_ruleEditAct->setEnabled(valid);
	this->m_ruleDeleteAct->setEnabled(valid);

	QMenu menu(this);
	menu.addAction(this->m_ruleNewAct);
	menu.addAction(this->m_ruleEditAct);
	menu.addAction(this->m_ruleDeleteAct);
	menu.exec(this->m_ruleView->viewport()->mapToGlobal(pos));
}

void CMainWindow::onAcceptError(QAbstractSocket::SocketError socketError)
{
	qWarning() << "acceptError" << socketError;
}

void CMainWindow::onNewConnection()
{
	QTcpSocket *socket = this->m_tcpServer->nextPendingConnection();
	if (!socket)
	{
		// 0 is returned if this function is called when there are no pending connections.
		return;
	}

	auto client = QSharedPointer<CClientSocket>(new CClientSocket(socket), &QObject::deleteLater);
	this->m_sockets.push_back(socket);
	this->m_clients.insert(socket, client);

	QObject::connect(socket, &QTcpSocket::disconnected, this, &CMainWindow::onDisconnected);
	QObject::connect(socket, &QAbstractSocket::errorOccurred, this, &CMainWindow::onError);
	QObject::connect(client.data(), &CClientSocket::updated, this, &CMainWindow::onClientUpdated);

	// Show on view
	this->m_clientModel->appendClient(client);
	this->statusBar()->showMessage(tr("Total client: %1").arg(this->m_sockets.size()));
}

void CMainWindow::onDisconnected()
{
	QTcpSocket *socket = static_cast<QTcpSocket *>(sender());
	this->m_sockets.removeOne(socket);

	// DO NOT TOUCH socket
	// socket->deleteLater();

	// Remove from view
	auto it = this->m_clients.find(socket);
	if (it == this->m_clients.end())
		return;

	auto client = it.value();
	client->terminate();
	this->m_clients.erase(it);
	this->m_clientModel->removeClient(client);
	this->statusBar()->showMessage(tr("Total client: %1").arg(this->m_sockets.size()));
}

void CMainWindow::onError(QAbstractSocket::SocketError)
{
	QTcpSocket *socket = static_cast<QTcpSocket *>(this->sender());
	auto it = this->m_clients.find(socket);
	if (it == this->m_clients.end())
	{
		socket->disconnectFromHost();
		return;
	}
}

void CMainWindow::onClientUpdated()
{
	CClientSocket *client = qobject_cast<CClientSocket *>(this->sender());
	this->m_clientModel->updateClient(client);
}

void CMainWindow::loadSettings()
{
	QSettings settings;
	this->restoreState(settings.value("windowState").toByteArray());
	this->restoreGeometry(settings.value("windowGeometry").toByteArray());
	this->m_clientView->horizontalHeader()->restoreState(settings.value("view1").toByteArray());
	this->m_ruleView->horizontalHeader()->restoreState(settings.value("view2").toByteArray());

	const int size = settings.beginReadArray("rules");
	for (int i = 0; i < size; i++)
	{
		settings.setArrayIndex(i);
		auto ruleSet = QSharedPointer<CRuleSet>::create();
		ruleSet->restoreState(settings.value("rule").toByteArray());
		this->m_ruleModel->append(ruleSet);
	}
	settings.endArray();
}

void CMainWindow::saveSettings()
{
	QSettings settings;
	settings.setValue("windowState", this->saveState());
	settings.setValue("windowGeometry", this->saveGeometry());
	settings.setValue("view1", this->m_clientView->horizontalHeader()->saveState());
	settings.setValue("view2", this->m_ruleView->horizontalHeader()->saveState());

	settings.beginWriteArray("rules");
	for (int row = 0; row < this->m_ruleModel->rowCount(); row++)
	{
		const auto ruleSet = this->m_ruleModel->get(row);
		settings.setArrayIndex(row);
		settings.setValue("rule", ruleSet->saveState());
	}
	settings.endArray();
}
