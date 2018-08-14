#include"osp.h"
#include"client.h"
#if _LINUX_
#include"list.h"
#endif
#include"demogui.h"
#include<QByteArray>
#include <QtWidgets/QFrame>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QLabel>
#include <QtWidgets/QToolButton>
#include<QDir>

#include<QTimer>

#define MAX_MESSAGE_LENGTH    1204
static GuiApp* g_GuiApp;
static demoCInstance* ins;
API void Connect2Server();

fileFrame::fileFrame(QWidget*parent,u16 wfileNum,LPCSTR fileName)
        : QFrame(parent)
{
        LPSTR p;
        s8 wFileName_NP[MAX_FILE_NAME_LENGTH];

        if((p = strrchr((LPSTR)fileName,'\\'))){
                strcpy((LPSTR)wFileName_NP,p+1);
        }else{
                strcpy((LPSTR)wFileName_NP,(LPCSTR)fileName);
        }


        strcpy((LPSTR)m_wFileName,(LPCSTR)fileName);
        setGeometry(QRect(0,80 *(wfileNum++), 191, 80));
        setFrameShape(QFrame::StyledPanel);
        setFrameShadow(QFrame::Plain);
        setLineWidth(1);
        setMidLineWidth(0);
        Pb_FileSize = new QProgressBar(this);
        Pb_FileSize->setGeometry(QRect(5, 53, 181, 21));
        Pb_FileSize->setValue(0);
        Lb_FileName = new QLabel(this);
        Lb_FileName->setGeometry(QRect(10, 10, 81, 16));
        Lb_FileName->setText(wFileName_NP);
        TBtn_GoOn = new QToolButton(this);
        TBtn_GoOn->setGeometry(QRect(10, 30, 37, 18));
        TBtn_GoOn->setPopupMode(QToolButton::InstantPopup);
        TBtn_GoOn->setToolButtonStyle(Qt::ToolButtonFollowStyle);
        TBtn_GoOn->setArrowType(Qt::LeftArrow);
        TBtn_Cancel= new QToolButton(this);
        TBtn_Cancel->setGeometry(QRect(60, 30, 37, 18));
        TBtn_Cancel->setText("Suspended");
        TBtn_Remove = new QToolButton(this);
        TBtn_Remove->setGeometry(QRect(110, 30, 37, 18));
        TBtn_Remove->setText("Remove");

}
void fileFrame::FileCancel(){

        if(OSP_OK != ::OspPost(MAKEIID(CLIENT_APP_ID,CInstance::DAEMON),SEND_CANCEL_CMD,
                        m_wFileName,strlen((LPCSTR)m_wFileName)+1)){
               OspLog(LOG_LVL_ERROR,"[SignIn] post error\n");
               return;
        }
}

void fileFrame::FileGoOn(){

        if(OSP_OK != ::OspPost(MAKEIID(CLIENT_APP_ID,CInstance::DAEMON),FILE_GO_ON_CMD,
                        m_wFileName,strlen((LPCSTR)m_wFileName)+1)){
               OspLog(LOG_LVL_ERROR,"[SignIn] post error\n");
               return;
        }
}

void fileFrame::FileRemove(){

        if(OSP_OK != ::OspPost(MAKEIID(CLIENT_APP_ID,CInstance::DAEMON),SEND_REMOVE_CMD,
                        m_wFileName,strlen((LPCSTR)m_wFileName)+1)){
               OspLog(LOG_LVL_ERROR,"[SignIn] post error\n");
               return;
        }
}


demogui::demogui(QWidget *parent)
	: QWidget(parent)
    ,wFileNum(0)
{
        LB_SignState = new QLabel(this);
        LB_SignState->setGeometry(QRect(230,10,113,20));
        LB_SignState->setText("UnSigned");
        qFileDialog = new QFileDialog(this);
        qFileDialog->setFileMode(QFileDialog::ExistingFiles);
        qFileDialog->setViewMode(QFileDialog::List);

        ui.setupUi(this);
        ui.verticalScrollBar->hide();
        connect(ui.Btn_SignIn,SIGNAL(clicked()),this,SLOT(SignIn()));
        connect(ui.Btn_SignOut,SIGNAL(clicked()),this,SLOT(SignOut()));
        connect(ui.Btn_Upload,SIGNAL(clicked()),this,SLOT(FileDialogShow()));
        connect(qFileDialog,SIGNAL(accepted()),this,SLOT(FileUpload()));
        connect(ui.verticalScrollBar,SIGNAL(sliderMoved(int)),this,SLOT(S_SliderMoved(int)));
        connect(ins,SIGNAL(SignInAck()),this,SLOT(SignInShow()));
        connect(ins,SIGNAL(SignOutAck()),this,SLOT(SignOutShow()));
        connect(ins,SIGNAL(FileSizeAck(TGuiAck*)),this,SLOT(FileSizeShow(TGuiAck*)));
        connect(ins,SIGNAL(UploadFileSizeAck(TGuiAck*)),this,SLOT(UploadFileSizeShow(TGuiAck*)));
        connect(ins,SIGNAL(FileFinishAck(TGuiAck*)),this,SLOT(FileFinishShow(TGuiAck*)));
        connect(ins,SIGNAL(FileUploadAck(TGuiAck*)),this,SLOT(FileUploadShow(TGuiAck*)));
        connect(ins,SIGNAL(FileCancelAck(TGuiAck*)),this,SLOT(FileCancelShow(TGuiAck*)));
        connect(ins,SIGNAL(FileGoOnAck(TGuiAck*)),this,SLOT(FileGoOnShow(TGuiAck*)));
        connect(ins,SIGNAL(FileRemoveAck(TGuiAck*)),this,SLOT(FileRemoveShow(TGuiAck*)));
}

void demogui::SignInShow(){
        LB_SignState->setText("Signed");
        ui.TE_ACKShow->append(QString("Sign In"));
}

void demogui::SignOutShow(){
        LB_SignState->setText("UnSigned");
        ui.TE_ACKShow->append(QString("Sign Out"));
}


void demogui::FileSizeShow(TGuiAck* tGuiAck){

        char mes[MAX_MESSAGE_LENGTH];

        if(tGuiAck->wGuiAck != 0){
              sprintf(mes,"file upload error:%d",tGuiAck->wGuiAck);
              ui.TE_ACKShow->append(QString((LPCSTR)mes));
              return;
        }
        OspLog(SYS_LOG_LEVEL,"FileSizeShow:%s\n",tGuiAck->FileName);

        tFileFrame[wFileNum] = new fileFrame(this,wFileNum,(LPCSTR)tGuiAck->FileName);
        if(!tFileFrame[wFileNum]){
              sprintf(mes,"file upload error:%d",tGuiAck->wGuiAck);
              ui.TE_ACKShow->append(QString((LPCSTR)mes));
              return;
        }
        connect(tFileFrame[wFileNum]->TBtn_Cancel,SIGNAL(clicked()),tFileFrame[wFileNum]
                        ,SLOT(FileCancel()));
        connect(tFileFrame[wFileNum]->TBtn_GoOn,SIGNAL(clicked()),tFileFrame[wFileNum]
                        ,SLOT(FileGoOn()));
        connect(tFileFrame[wFileNum]->TBtn_Remove,SIGNAL(clicked()),tFileFrame[wFileNum]
                        ,SLOT(FileRemove()));
        tFileFrame[wFileNum]->show();
        if(80*(wFileNum+1) > ui.frame->height()){
                ui.verticalScrollBar->show();
                ui.verticalScrollBar->setMaximum((wFileNum+1)*80-400);
                ui.verticalScrollBar->setValue((wFileNum+1)*80-400);
                ui.verticalScrollBar->setMinimum(0);
        }

        tFileFrame[wFileNum]->Pb_FileSize->setMaximum(tGuiAck->dwFileSize);
        tFileFrame[wFileNum]->Pb_FileSize->setMinimum(0);
        wFileNum++;
        delete tGuiAck;
#if 0
        for(u16 i = 0;i < wFileNum;i++){
                if(strcmp((LPCSTR)tFileFrame[i]->m_wFileName,(LPCSTR)tGuiAck->FileName) == 0){
                        tFileFrame[i]->Pb_FileSize->setMaximum(tGuiAck->dwFileSize);
                        tFileFrame[i]->Pb_FileSize->setMinimum(0);
                        break;
                }
        }
#endif
}

void demogui::UploadFileSizeShow(TGuiAck* tGuiAck){

        for(u16 i = 0;i < wFileNum;i++){
                if(strcmp((LPCSTR)tFileFrame[i]->m_wFileName,(LPCSTR)tGuiAck->FileName) == 0){
                        if(!tFileFrame[i]->isEnabled()){
                                continue;
                        }
                        tFileFrame[i]->Pb_FileSize->setValue(tGuiAck->dwUploadFileSize);
                        break;
                }
        }
}

void demogui::FileFinishShow(TGuiAck* tGuiAck){

        u8 mes[MAX_MESSAGE_LENGTH];
        LPSTR p;

        if((p = strrchr((LPSTR)tGuiAck->FileName,'\\'))){
                strcpy((LPSTR)mes,p+1);
        }else{
                strcpy((LPSTR)mes,(LPCSTR)tGuiAck->FileName);
        }

        strcat((LPSTR)mes," upload finished");
        ui.TE_ACKShow->append(QString((LPCSTR(mes))));
}

void demogui::FileCancelShow(TGuiAck* tGuiAck){

        u8 mes[MAX_MESSAGE_LENGTH];
        LPSTR p;

        if((p = strrchr((LPSTR)tGuiAck->FileName,'\\'))){
                strcpy((LPSTR)mes,p+1);
        }else{
                strcpy((LPSTR)mes,(LPCSTR)tGuiAck->FileName);
        }

        strcat((LPSTR)mes," suspended");
        ui.TE_ACKShow->append(QString((LPCSTR(mes))));
}

void demogui::FileRemoveShow(TGuiAck* tGuiAck){

        u8 mes[MAX_MESSAGE_LENGTH];
        LPSTR p;

        if(tGuiAck->wGuiAck != 0){
              sprintf((LPSTR)mes,"file Remove error:%d",tGuiAck->wGuiAck);
              ui.TE_ACKShow->append(QString((LPCSTR)mes));
              return;
        }

        for(u16 i = 0;i < wFileNum;i++){
                if(strcmp((LPCSTR)tFileFrame[i]->m_wFileName,(LPCSTR)tGuiAck->FileName) == 0){
                        if(!tFileFrame[i]->isEnabled()){
                                continue;
                        }
                        tFileFrame[i]->setEnabled(false);
                        break;
                }
        }

        if((p = strrchr((LPSTR)tGuiAck->FileName,'\\'))){
                strcpy((LPSTR)mes,p+1);
        }else{
                strcpy((LPSTR)mes,(LPCSTR)tGuiAck->FileName);
        }

        strcat((LPSTR)mes," Removed");
        ui.TE_ACKShow->append(QString((LPCSTR(mes))));
}

void demogui::FileGoOnShow(TGuiAck* tGuiAck){

        u8 mes[MAX_MESSAGE_LENGTH];
        LPSTR p;

        if((p = strrchr((LPSTR)tGuiAck->FileName,'\\'))){
                strcpy((LPSTR)mes,p+1);
        }else{
                strcpy((LPSTR)mes,(LPCSTR)tGuiAck->FileName);
        }

        strcat((LPSTR)mes," go on uploading");
        ui.TE_ACKShow->append(QString((LPCSTR(mes))));
}

void demogui::FileUploadShow(TGuiAck* tGuiAck){

        ui.TE_ACKShow->append(QString::number(tGuiAck->wGuiAck));
}

demoCInstance::demoCInstance(QObject* parent)
    : QObject(parent)
    ,m_wServerPort(SERVER_PORT)
    ,wFileNum(0)
{
        memcpy(m_byServerIp,SERVER_IP,sizeof(SERVER_IP));
        m_tCmdChain = NULL;
        m_tCmdDaemonChain = NULL;
        MsgProcessInit();
}


void demoCInstance::MsgProcessInit(){

        RegMsgProFun(MAKEESTATE(IDLE_STATE,GUI_SIGN_IN_ACK),&demoCInstance::GetSignIn,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,GUI_SIGN_OUT_ACK),&demoCInstance::GetSignOut,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,GUI_FILE_SIZE_ACK),&demoCInstance::GetFileSize,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,GUI_UPLOAD_FILE_SIZE_ACK),&demoCInstance::GetUploadFileSize,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,GUI_DISCONNECT),&demoCInstance::GetDisconnect,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,GUI_FILE_UPLOAD_ACK),&demoCInstance::GetFileUpload,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,GUI_FILE_FINISHED_ACK),&demoCInstance::GetFileFinished,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,GUI_FILE_CANCEL_ACK),&demoCInstance::GetFileCancel,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,GUI_FILE_REMOVE_ACK),&demoCInstance::GetFileRemove,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,GUI_FILE_GO_ON_ACK),&demoCInstance::GetFileGoOn,&m_tCmdDaemonChain);
}

void demoCInstance::GetSignIn(CMessage* const pMsg){
        
        u16 ack = *(s16*)pMsg->content;
        if(ack == 0){
              emit SignInAck();
        }
}

void demoCInstance::GetSignOut(CMessage* const pMsg){

        u16 ack = *(s16*)pMsg->content;
        if(ack == 0){
              emit SignOutAck();
        }

}

void demoCInstance::GetFileSize(CMessage* const pMsg){

        TGuiAck* tGuiAck;

        tGuiAck = new TGuiAck();
        memcpy(tGuiAck,pMsg->content,pMsg->length);
        emit FileSizeAck(tGuiAck);
}

void demoCInstance::GetUploadFileSize(CMessage* const pMsg){
        TGuiAck *tGuiAck;
        tGuiAck = (TGuiAck*)pMsg->content;

        emit UploadFileSizeAck(tGuiAck);
}

void demoCInstance::GetDisconnect(CMessage* const pMsg){
}

void demoCInstance::GetFileUpload(CMessage* const pMsg){
        TGuiAck *tGuiAck;
        tGuiAck = (TGuiAck*)pMsg->content;
        emit FileUploadAck(tGuiAck);
}

void demoCInstance::GetFileFinished(CMessage* const pMsg){

        emit FileFinishAck((TGuiAck*)pMsg->content);
}

void demoCInstance::GetFileCancel(CMessage* const pMsg){

        emit FileCancelAck((TGuiAck*)pMsg->content);
}

void demoCInstance::GetFileRemove(CMessage* const pMsg){

        emit FileRemoveAck((TGuiAck*)pMsg->content);
}

void demoCInstance::GetFileGoOn(CMessage* const pMsg){

        emit FileGoOnAck((TGuiAck*)pMsg->content);
}

void demogui::SignIn(){

        TSinInfo tSinInfo;

//        Connect2Server();

        strcpy(tSinInfo.Username,ui.LE_User->text().toLocal8Bit());
        strcpy(tSinInfo.Passwd,ui.LE_Pwd->text().toLocal8Bit());

        if(OSP_OK != ::OspPost(MAKEIID(CLIENT_APP_ID,CInstance::DAEMON),SIGN_IN_CMD,
                        &tSinInfo,sizeof(tSinInfo))){
               OspLog(LOG_LVL_ERROR,"[SignIn] post error\n");
               return;
        }
}

void demogui::SignOut(){

        if(OSP_OK != ::OspPost(MAKEIID(CLIENT_APP_ID,CInstance::DAEMON),SIGN_OUT_CMD)){
               OspLog(LOG_LVL_ERROR,"[SendSignOutCmd] post error\n");
        }

}

#if 0
void demogui::FileGoOn(){
}

#endif

void demogui::FileUpload(){

        QStringList fileNames;
        fileNames = qFileDialog->selectedFiles();
        QByteArray qbFileNames[MAX_FILE_NUM];
        u8 wFileName[MAX_FILE_NAME_LENGTH];

        for(int i = 0;i < fileNames.count();i++){
                fileNames[i] = QDir::toNativeSeparators(fileNames[i]);
                qbFileNames[i] = fileNames[i].toLocal8Bit();
                strcpy((LPSTR)wFileName,(LPSTR)qbFileNames[i].data());

                if(OSP_OK !=::OspPost(MAKEIID(CLIENT_APP_ID,CInstance::DAEMON),FILE_UPLOAD_CMD,
                                wFileName,strlen((LPCSTR)wFileName)+1)){
                       OspLog(LOG_LVL_ERROR,"[FileUpload] post error\n");
					   return;
                }
                OspLog(SYS_LOG_LEVEL,"fileUpload:%s\n",wFileName);

#if 0
                tFileFrame[i+wFileNum] = new fileFrame(this,wFileNum,(LPCSTR)wFileName);
                if(!tFileFrame[i+wFileNum]){
                        return;
                }
                connect(tFileFrame[i+wFileNum]->TBtn_Cancel,SIGNAL(clicked()),tFileFrame[i+wFileNum]
                                ,SLOT(FileCancel()));
                connect(tFileFrame[i+wFileNum]->TBtn_GoOn,SIGNAL(clicked()),tFileFrame[i+wFileNum]
                                ,SLOT(FileGoOn()));
                connect(tFileFrame[i+wFileNum]->TBtn_Remove,SIGNAL(clicked()),tFileFrame[i+wFileNum]
                                ,SLOT(FileRemove()));
                tFileFrame[i+wFileNum]->show();
				wFileNum++;
                if(80*wFileNum > ui.frame->height()){
                        ui.verticalScrollBar->show();
                        ui.verticalScrollBar->setMaximum(wFileNum*80-400);
                        ui.verticalScrollBar->setMinimum(0);
                }
#endif

        }
}

void demogui::S_SliderMoved(int value){
     
	int a = value;
	int max = ui.verticalScrollBar->maximum();
	int min = ui.verticalScrollBar->minimum();
    int nowvalue = ui.verticalScrollBar->value();

     QPoint point; 
#if 1
        for(int i = 0;i < wFileNum;i++){
                point = tFileFrame[i]->pos();
                tFileFrame[i]->move(0,i*80-nowvalue);
        }
#endif
}

void demogui::FileDialogShow(){

        qFileDialog->exec();
       // qFileDialog->show();
}

#if _LINUX_
typedef  void (*msgProcess)(CMessage*const);

typedef struct tagInsNode{
        struct list_head        tListHead;
        u32                     EventState;
        msgProcess  c_MsgProcess;
        struct      tagCmdNode *next;
}TInsNode;

static struct list_head  TInsNodeHead;
static TInsNode* findProcess(const u32 eventState,const struct list_head* tInsNodeHead);
static void regProcess(const u32 eventState,const msgProcess c_MsgProcess
                                ,struct list_head* tInsNodeHead);
static void delInsNode(struct list_head * tInsNodeHead);

void msgProcessInit();
#endif


int myOspInit(){

#ifdef _MSC_VER
        int ret = OspInit(TRUE,2500,"WindowsOspClient");
#else
        int ret = OspInit(TRUE,2500,"LinuxOspClient");
#endif
        bool bCreateTcpNodeFlag = false;
        u16 i;

        if(!ret){
                OspPrintf(1,0,"osp init fail\n");
                OspQuit();
                return -1;
        }
        OspSetPrompt("Client");
#if _LINUX_
        INIT_LIST_HEAD(&TInsNodeHead);
        msgProcessInit();
#endif

        if(!(g_GuiApp = new GuiApp())){
                OspLog(LOG_LVL_ERROR,"[myOspInit]app malloc error\n");
                return -1;
        }
        if(OSP_OK != g_GuiApp->CreateApp("GuiApp",GUI_APP_ID,GUI_APP_PRI,MAX_MSG_WAITING)){
                OspLog(LOG_LVL_ERROR,"[myOspInit]app create error\n");
#if _LINUX
                delInsNode(&TInsNodeHead);
#endif
                OspQuit();
                return -2;
        }
        ins = (demoCInstance*)((CApp*)g_GuiApp)->GetInstance(CInstance::DAEMON);
        if(!ins){
                OspLog(LOG_LVL_ERROR,"[myOspInit]get instance failed\n");
                return -5;
        }
        //尝试多次建立node，支持本地多个客户端的副本
        for(i = 0;i < CREATE_TCP_NODE_TIMES;i++){
                if(INVALID_SOCKET != (ret = OspCreateTcpNode(0,DEMO_GUI_LISTEN_PORT+i*3))){
                        bCreateTcpNodeFlag = true;
                        break;
                }
        }
        if(!bCreateTcpNodeFlag){
                OspQuit();
                OspLog(SYS_LOG_LEVEL,"[myOspInit]create positive node failed,quit\n");
                return -3;
        }


        if(0 != clientInit(DEMO_GUI_LISTEN_PORT+i*3)){
                OspLog(LOG_LVL_ERROR,"[myOspInit]client init error\n");
#if _LINUX_
                delInsNode(&TInsNodeHead);
#endif
                OspQuit();
                return -4;
        }

#if _LINUX_ 
        delInsNode(&TInsNodeHead);
#endif
//        OspQuit();

        return 0;
}




void GetSignIn(CMessage* const pMsg){
}

void GetSignOut(CMessage* const pMsg){
        printf("[GetSignOut]ack:%d\n",*(s16*)pMsg->content);
}

static  u32 file_size;
void GetFileSize(CMessage* const pMsg){
        printf("[GetFileSize]ack:%ld\n",*(u32*)pMsg->content);
        file_size = *(u32*)pMsg->content;
}

void GetUploadFileSize(CMessage* const pMsg){

        u32 uploadfilesize = *(u32*)pMsg->content;
        printf("[GetUploadFileSize]ack:%f\n",(float)uploadfilesize/(float)file_size);
}

void GetDisconnect(CMessage* const pMsg){
        printf("[GetDisconnect]ack:%d\n",*(s16*)pMsg->content);
}

void GetFileUpload(CMessage* const pMsg){
        printf("[GetFileUpload]ack:%d\n",*(s16*)pMsg->content);
}

void GetFileFinished(CMessage* const pMsg){
        printf("[GetFileFinished]ack:%d\n",*(s16*)pMsg->content);
}

void GetFileCancel(CMessage* const pMsg){
        printf("[GetFileCancel]ack:%d\n",*(s16*)pMsg->content);
}

void GetFileRemove(CMessage* const pMsg){
        printf("[GetFileRemove]ack:%d\n",*(s16*)pMsg->content);
}

void GetFileGoOn(CMessage* const pMsg){
        printf("[GetFileGoON]ack:%d\n",*(s16*)pMsg->content);
}

#if _LINUX_

void demogui::InstanceEntry(CMessage *const pMsg){

        u32 curState = CurState();
        u16 curEvent = pMsg->event;
        msgProcess c_MsgProcess;
        TInsNode *fInsNode;

        if(NULL == pMsg){
                OspLog(LOG_LVL_ERROR,"[InstanceEntry]msg is NULL\n");
                return;
        }

        if((fInsNode = findProcess(MAKEESTATE(curState,curEvent),&TInsNodeHead))){
                fInsNode->c_MsgProcess(pMsg);
        }else{
                OspLog(LOG_LVL_ERROR,"[InstanceEntry]state or event error\n");
        }
}

void demogui::DaemonInstanceEntry(CMessage *const pMsg,CApp* pApp){

        u32 curState = CurState();
        u16 curEvent = pMsg->event;
        msgProcess c_MsgProcess;
        TInsNode *fInsNode;

        if(NULL == pMsg){
                OspLog(LOG_LVL_ERROR,"[InstanceEntry]msg is NULL\n");
                return;
        }

        if((fInsNode = findProcess(MAKEESTATE(curState,curEvent),&TInsNodeHead))){
                fInsNode->c_MsgProcess(pMsg);
        }else{
                OspLog(LOG_LVL_ERROR,"[InstanceEntry]state or event error,event:%d, \
                                state:%d\n",curEvent,curState);
        }
}

void msgProcessInit(){

        regProcess(MAKEESTATE(IDLE_STATE,GUI_SIGN_IN_ACK),GetSignIn,&TInsNodeHead);
        regProcess(MAKEESTATE(IDLE_STATE,GUI_SIGN_OUT_ACK),GetSignOut,&TInsNodeHead);
        regProcess(MAKEESTATE(IDLE_STATE,GUI_FILE_SIZE_ACK),GetFileSize,&TInsNodeHead);
        regProcess(MAKEESTATE(IDLE_STATE,GUI_UPLOAD_FILE_SIZE_ACK),GetUploadFileSize,&TInsNodeHead);
        regProcess(MAKEESTATE(IDLE_STATE,GUI_DISCONNECT),GetDisconnect,&TInsNodeHead);
        regProcess(MAKEESTATE(IDLE_STATE,GUI_FILE_UPLOAD_ACK),GetFileUpload,&TInsNodeHead);
        regProcess(MAKEESTATE(IDLE_STATE,GUI_FILE_FINISHED_ACK),GetFileFinished,&TInsNodeHead);
        regProcess(MAKEESTATE(IDLE_STATE,GUI_FILE_CANCEL_ACK),GetFileCancel,&TInsNodeHead);
        regProcess(MAKEESTATE(IDLE_STATE,GUI_FILE_REMOVE_ACK),GetFileRemove,&TInsNodeHead);
        regProcess(MAKEESTATE(IDLE_STATE,GUI_FILE_GO_ON_ACK),GetFileGoOn,&TInsNodeHead);
}

static TInsNode* findProcess(const u32 eventState,const struct list_head* tInsNodeHead){

        TInsNode *tnInsNode;
        struct list_head *insHead;

        list_for_each(insHead,tInsNodeHead){
                tnInsNode = list_entry(insHead,TInsNode,tListHead);
                if(tnInsNode->EventState == eventState){
                        return tnInsNode;
                }
                
        }
        return NULL;
}

static void regProcess(const u32 eventState,const msgProcess c_MsgProcess
                                ,struct list_head* tInsNodeHead){

        TInsNode* tInsNode = NULL;

        if(!(tInsNode = new TInsNode())){
                OspLog(LOG_LVL_ERROR,"[regProcess]node malloc error\n");
                return;
        }

        if(findProcess(eventState,tInsNodeHead)){
                OspLog(LOG_LVL_ERROR,"[regProcess]node already registered\n");
                return;
        }

        tInsNode->c_MsgProcess = c_MsgProcess;
        tInsNode->EventState = eventState;
        list_add(&tInsNode->tListHead,tInsNodeHead);
}

static void delInsNode(struct list_head* tInsNodeHead){

        struct list_head *insHead,*templist;
        TInsNode *tnInsNode;

        list_for_each_safe(insHead,templist,tInsNodeHead){
                tnInsNode = list_entry(insHead,TInsNode,tListHead);
                list_del(&tnInsNode->tListHead);
                delete tnInsNode;
        }

        INIT_LIST_HEAD(tInsNodeHead);
}
#endif

#if _MSC_VER
bool demoCInstance::RegMsgProFun(u32 EventState,MsgProcess c_MsgProcess,tCmdNode** tppNodeChain){

        tCmdNode *Node,*NewNode,*LNode;

        Node = *tppNodeChain;

#if THREAD_SAFE_MALLOC
        if(!(NewNode = (tCmdNode*)malloc(sizeof(tCmdNode)))){
#else
        if(!(NewNode = new tCmdNode())){
#endif
                OspLog(LOG_LVL_ERROR,"[RegMsgProFun] node malloc error\n");
                return false;
        }

        NewNode->EventState = EventState;
        NewNode->c_MsgProcess = c_MsgProcess;
        NewNode->next = NULL;

        if(!Node){
                *tppNodeChain = NewNode;
                OspLog(SYS_LOG_LEVEL,"cmd chain init \n");
                return true;
        }

        while(Node){
                if(Node->EventState == EventState){
                        OspLog(LOG_LVL_ERROR,"[RegMsgProFun] node already in \n");
                        printf("[RegMsgProFun] node already in \n");
                        return false;
                }
                LNode = Node;
                Node = Node->next;
        }
        LNode->next = NewNode;

        return true;
}

bool demoCInstance::FindProcess(u32 EventState,MsgProcess* c_MsgProcess,tCmdNode* tNodeChain){

        tCmdNode *Node;

        Node = tNodeChain;
        if(!Node){
                OspLog(LOG_LVL_ERROR,"[FindProcess] Node Chain is NULL\n");
                printf("[FindProcess] Node Chain is NULL\n");
                return false;
        }
        while(Node){

                if(Node->EventState == EventState){
                        *c_MsgProcess = Node->c_MsgProcess;
                        return true;
                }
                Node = Node->next;
        }

        return false;
}

void demoCInstance::InstanceEntry(CMessage * const pMsg){

        u32 curState = CurState();
        u16 curEvent = pMsg->event;
        MsgProcess c_MsgProcess;

#if 0
        if(!m_bConnectedFlag){
                OspLog(LOG_LVL_ERROR,"[InstanceEntry]disconnected\n");
                return;
        }
        if(!m_bSignFlag){
                OspLog(SYS_LOG_LEVEL,"[InstanceEntry]sign out\n");
                return;
        }
#endif

        if(NULL == pMsg){
                OspLog(LOG_LVL_ERROR,"[InstanceEntry] pMsg is NULL\n");
                return;
        }

        if(FindProcess(MAKEESTATE(curState,curEvent),&c_MsgProcess,m_tCmdChain)){
                (this->*c_MsgProcess)(pMsg);
        }else{
                OspLog(LOG_LVL_ERROR,"[InstanceEntry] can not find the EState,event:%d,state:%d\n"
                                ,curEvent,curState);
                printf("[InstanceEntry] can not find the EState\n");
        }
}

void demoCInstance::DaemonInstanceEntry(CMessage *const pMsg,CApp *pCApp){

        u32 curState = CurState();
        u16 curEvent = pMsg->event;
        MsgProcess c_MsgProcess;

        if(NULL == pMsg){
                OspLog(LOG_LVL_ERROR,"[DaemonInstanceEntry] pMsg is NULL\n");
                return;
        }

        if(FindProcess(MAKEESTATE(curState,curEvent),&c_MsgProcess,m_tCmdDaemonChain)){
                (this->*c_MsgProcess)(pMsg);
        }else{
                OspLog(LOG_LVL_ERROR,"[DaemonInstanceEntry] can not find the EState,event:%d,state:%d\n"
                                ,curEvent,curState);
                printf("[DaemonInstanceEntry] can not find the EState\n");
				exit(-1);
        }
}


void demoCInstance::NodeChainEnd(){

        tCmdNode *tmpNode;

        while(m_tCmdChain){
                tmpNode = m_tCmdChain->next;
#if THREAD_SAFE_MALLOC
                free(m_tCmdChain);
#else
                delete m_tCmdChain;
#endif
                m_tCmdChain = tmpNode;
        }

        while(m_tCmdDaemonChain){
                tmpNode = m_tCmdDaemonChain->next;
#if THREAD_SAFE_MALLOC
                free(m_tCmdDaemonChain);
#else
                delete m_tCmdDaemonChain;
#endif
                m_tCmdDaemonChain = tmpNode;
        }
}

#endif