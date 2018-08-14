/********************************************************************************
** Form generated from reading UI file 'demogui.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEMOGUI_H
#define UI_DEMOGUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_demoguiClass
{
public:
    QPushButton *Btn_Upload;
    QTextEdit *TE_ACKShow;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *LE_IP;
    QLineEdit *LE_User;
    QVBoxLayout *verticalLayout;
    QLineEdit *LE_Port;
    QLineEdit *LE_Pwd;
    QSpacerItem *verticalSpacer_2;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *Btn_SignIn;
    QPushButton *Btn_SignOut;
    QFrame *frame;
    QScrollBar *verticalScrollBar;

    void setupUi(QWidget *demoguiClass)
    {
        if (demoguiClass->objectName().isEmpty())
            demoguiClass->setObjectName(QStringLiteral("demoguiClass"));
        demoguiClass->resize(486, 400);
        Btn_Upload = new QPushButton(demoguiClass);
        Btn_Upload->setObjectName(QStringLiteral("Btn_Upload"));
        Btn_Upload->setGeometry(QRect(230, 350, 91, 41));
        TE_ACKShow = new QTextEdit(demoguiClass);
        TE_ACKShow->setObjectName(QStringLiteral("TE_ACKShow"));
        TE_ACKShow->setGeometry(QRect(230, 160, 231, 151));
        layoutWidget = new QWidget(demoguiClass);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(230, 40, 221, 95));
        verticalLayout_3 = new QVBoxLayout(layoutWidget);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        LE_IP = new QLineEdit(layoutWidget);
        LE_IP->setObjectName(QStringLiteral("LE_IP"));

        verticalLayout_2->addWidget(LE_IP);

        LE_User = new QLineEdit(layoutWidget);
        LE_User->setObjectName(QStringLiteral("LE_User"));

        verticalLayout_2->addWidget(LE_User);


        horizontalLayout->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        LE_Port = new QLineEdit(layoutWidget);
        LE_Port->setObjectName(QStringLiteral("LE_Port"));

        verticalLayout->addWidget(LE_Port);

        LE_Pwd = new QLineEdit(layoutWidget);
        LE_Pwd->setObjectName(QStringLiteral("LE_Pwd"));

        verticalLayout->addWidget(LE_Pwd);


        horizontalLayout->addLayout(verticalLayout);


        verticalLayout_3->addLayout(horizontalLayout);

        verticalSpacer_2 = new QSpacerItem(20, 88, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        Btn_SignIn = new QPushButton(layoutWidget);
        Btn_SignIn->setObjectName(QStringLiteral("Btn_SignIn"));

        horizontalLayout_2->addWidget(Btn_SignIn);

        Btn_SignOut = new QPushButton(layoutWidget);
        Btn_SignOut->setObjectName(QStringLiteral("Btn_SignOut"));

        horizontalLayout_2->addWidget(Btn_SignOut);


        verticalLayout_3->addLayout(horizontalLayout_2);

        frame = new QFrame(demoguiClass);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setGeometry(QRect(0, 0, 191, 400));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Sunken);
        verticalScrollBar = new QScrollBar(demoguiClass);
        verticalScrollBar->setObjectName(QStringLiteral("verticalScrollBar"));
        verticalScrollBar->setGeometry(QRect(191, 0, 16, 400));
        verticalScrollBar->setOrientation(Qt::Vertical);

        retranslateUi(demoguiClass);

        QMetaObject::connectSlotsByName(demoguiClass);
    } // setupUi

    void retranslateUi(QWidget *demoguiClass)
    {
        demoguiClass->setWindowTitle(QApplication::translate("demoguiClass", "File Upload", 0));
        Btn_Upload->setText(QApplication::translate("demoguiClass", "File Upload", 0));
        LE_IP->setText(QApplication::translate("demoguiClass", "172.16.160.94", 0));
        LE_User->setText(QApplication::translate("demoguiClass", "Robert", 0));
        LE_Port->setText(QApplication::translate("demoguiClass", "2000", 0));
        LE_Pwd->setText(QApplication::translate("demoguiClass", "admin", 0));
        Btn_SignIn->setText(QApplication::translate("demoguiClass", "Sign In", 0));
        Btn_SignOut->setText(QApplication::translate("demoguiClass", "Sign Out", 0));
    } // retranslateUi

};

namespace Ui {
    class demoguiClass: public Ui_demoguiClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEMOGUI_H
