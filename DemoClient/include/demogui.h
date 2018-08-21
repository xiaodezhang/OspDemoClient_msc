#ifndef DEMOGUI_H
#define DEMOGUI_H

#include <QtWidgets/QWidget>
#include<QFileDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QToolButton>
#include "ui_demogui.h"
#include"ui_serversettings.h"
#include"client.h"
#include"commondemo.h"
API int myOspInit();

#define DEMO_GUI_LISTEN_PORT                    5000
#define GUI_APP_ID                              (u16)4
#define GUI_APP_PRI                             (u8)80
#define CREATE_TCP_NODE_TIMES     20

#define GUI_MSG_BASE                            (u16)0xf111

#define GUI_SIGN_IN_ACK                         (GUI_MSG_BASE+1)
#define GUI_SIGN_OUT_ACK                        (GUI_MSG_BASE+2)
#define GUI_FILE_SIZE_ACK                       (GUI_MSG_BASE+3)
#define GUI_UPLOAD_FILE_SIZE_ACK                (GUI_MSG_BASE+4)
#define GUI_DISCONNECT                          (GUI_MSG_BASE+5)
#define GUI_FILE_UPLOAD_ACK                     (GUI_MSG_BASE+6)
#define GUI_FILE_FINISHED_ACK                   (GUI_MSG_BASE+7)
#define GUI_FILE_CANCEL_ACK                     (GUI_MSG_BASE+8)
#define GUI_FILE_REMOVE_ACK                     (GUI_MSG_BASE+9)
#define GUI_FILE_GO_ON_ACK                      (GUI_MSG_BASE+10)

#define MAX_FILE_NUM                             200

class fileFrame: public QFrame{

	Q_OBJECT

public:
    fileFrame(QWidget *parent = 0,u16 wfileNum = 0,LPCSTR fileName = "",u16 moveFlag = 0);
    ~fileFrame(){}

public:
    QProgressBar *Pb_FileSize;
    QLabel *Lb_FileName;
    QToolButton *TBtn_GoOn,*TBtn_Cancel,*TBtn_Remove;
    unsigned char m_wFileName[MAX_FILE_NAME_LENGTH];
    u16           m_wMoveFlag;
    u16           m_wMoveStep;
public slots:
    void FileCancel();
    void FileGoOn();
    void FileRemove();

};

class ServerSettings : public QWidget{

    Q_OBJECT
private:
	Ui::FM_ServerSettings FM_ui;
public:
    ServerSettings(QWidget*parent = 0);
    ~ServerSettings(){}
    QByteArray ServerSettings::GetIP();
    int GetPort();
public slots:
    void SettingsCancel();
    void SettingsAccept();
};

class demogui : public QWidget
{
	Q_OBJECT

public:
	demogui(QWidget *parent = 0);
    ~demogui(){}


private:
	Ui::demoguiClass ui;
public:
        void GetSignIn(CMessage*const);
        void GetSignOut(CMessage*const);
        void GetFileSize(CMessage*const);
        void GetUploadFileSize(CMessage*const);
        void GetDisconnect(CMessage*const);
        void GetFileUpload(CMessage*const);
        void GetFileFinished(CMessage*const);
        void GetFileCancel(CMessage*const);
        void GetFileRemove(CMessage*const);
        void GetFileGoOn(CMessage*const);
private:
        void createFileFrame(LPCSTR);
        u16  wFileNum;

public slots:
        void SignIn();
        void SignOut();
        void SettingsShow();
        void FileUpload();



        void FileDialogShow();
        void S_SliderMoved(int);
        void SignInShow();
        void SignOutShow();
        void FileSizeShow(TGuiAck*);
        void UploadFileSizeShow(TGuiAck*);
        void FileUploadShow(TGuiAck*);
        void FileFinishShow(TGuiAck*);
        void FileCancelShow(TGuiAck*);
        void FileGoOnShow(TGuiAck*);
        void FileRemoveShow(TGuiAck*);
        void DisconnectShow();

private: 
        QFileDialog *qFileDialog;
        fileFrame * tFileFrame[MAX_FILE_NUM];
        QLabel* LB_SignState;


};

class demoCInstance: public QObject,public CInstance
{
	Q_OBJECT

public:
	demoCInstance(QObject*parent = 0);
    ~demoCInstance(){
    NodeChainEnd();
}

    void MsgProcessInit();
private:
        u8          m_byServerIp[MAX_IP_LENGTH];
        u16         m_wServerPort;
private:
        void InstanceEntry(CMessage *const);
        void DaemonInstanceEntry(CMessage *const,CApp*);
public:
        typedef void (demoCInstance::*MsgProcess)(CMessage *const pMsg);
        typedef struct tagCmdNode{
                u32         EventState;
                demoCInstance::MsgProcess  c_MsgProcess;
                struct      tagCmdNode *next;
        }tCmdNode;

        void NodeChainEnd();
        bool RegMsgProFun(u32,MsgProcess,tCmdNode**);
        bool FindProcess(u32,MsgProcess*,tCmdNode*);
        void GetSignIn(CMessage*const);
        void GetSignOut(CMessage*const);
        void GetFileSize(CMessage*const);
        void GetUploadFileSize(CMessage*const);
        void GetDisconnect(CMessage*const);
        void GetFileUpload(CMessage*const);
        void GetFileFinished(CMessage*const);
        void GetFileCancel(CMessage*const);
        void GetFileRemove(CMessage*const);
        void GetFileGoOn(CMessage*const);
signals:
       void SignInAck();
       void SignOutAck();
       void FileSizeAck(TGuiAck*);
       void UploadFileSizeAck(TGuiAck*);
       void FileFinishAck(TGuiAck*);
       void FileUploadAck(TGuiAck*);
       void FileCancelAck(TGuiAck*);
       void FileGoOnAck(TGuiAck*);
       void FileRemoveAck(TGuiAck*);
       void Disconnect();

private:
        u32 dwFileSize;
private: 
        tCmdNode *m_tCmdChain;
        tCmdNode *m_tCmdDaemonChain;
};

typedef zTemplate<demoCInstance,1,CAppNoData,MAX_ALIAS_LENGTH> GuiApp;

#endif // DEMOGUI_H
