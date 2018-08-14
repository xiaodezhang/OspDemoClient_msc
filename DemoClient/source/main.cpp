#include "demogui.h"
#include"osp.h"
#include <QtWidgets/QApplication>
#include"client.h"


int main(int argc, char *argv[])
{

	QApplication a(argc, argv);
	demogui *w;

    qRegisterMetaType<TGuiAck>("TGuiAck");
	myOspInit();
    w = new demogui();
	w->show();
	return a.exec();

}
