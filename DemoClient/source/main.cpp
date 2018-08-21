#include "demogui.h"
#include"osp.h"
#include <QtWidgets/QApplication>
#include"client.h"


ServerSettings* serverSettings;

int main(int argc, char *argv[])
{

	QApplication a(argc, argv);
	demogui *w;

    qRegisterMetaType<TGuiAck>("TGuiAck");
	myOspInit();
    w = new demogui();
    serverSettings = new ServerSettings();
    serverSettings->hide();
	w->show();
	return a.exec();

}
