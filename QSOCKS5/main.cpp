#include "stdafx.h"

#include "CMainWindow.hpp"
#include "Vars.hpp"

void MessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QString txt;
	switch (type)
	{
	case QtDebugMsg:
		txt = QString("Debug: %1").arg(msg);
		break;
	case QtWarningMsg:
		txt = QString("Warning: %1").arg(msg);
		break;
	case QtCriticalMsg:
		txt = QString("Critical: %1").arg(msg);
		break;
	case QtFatalMsg:
		txt = QString("Fatal: %1").arg(msg);
		break;
	case QtInfoMsg:
		txt = QString("Info: %1").arg(msg);
		break;
	}

	QFile file("qtlog.txt");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
		return;

	QTextStream stream(&file);
	stream << txt << Qt::endl;
}

int main(int argc, char *argv[])
{
	qInstallMessageHandler(MessageHandler);

	// application settings
	QCoreApplication::setOrganizationName("org");
	QCoreApplication::setApplicationName("QSOCKS5");

	QApplication app(argc, argv);
	CMainWindow window;
	g_mainWindow = &window;
	window.show();
	return app.exec();
}
