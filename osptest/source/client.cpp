#include"osp.h"
#include"client.h"
#include"demogui.h"
#include<algorithm>
#if _LINUX_
#include"list.h"
#else
#include<list>
#endif
#include"zlib.h"

#define CLIENT_APP_SUM           5
#define APP_NUM_SIZE             20

#define SERVER_DELAY             1000
#define MY_FILE_NAME             "mydoc.7z"
#define NATIVE_IP                "127.0.0.1"

#if 0
#define MAX_CMD_REPEAT_TIMES     5
#endif
using namespace std;

API void Test_DisConnect();
API void Test_Cancel();
API void Connect2Server();
API void SendSignInCmd();
API void SendSignOutCmd();
API void SendFileUploadCmd();
API void MultSendFileUploadCmd();
API void SendCancelCmd();
API void SendRemoveCmd();
API void SendFileGoOnCmd();
API void Disconnect2Server();

API int def(FILE *source, FILE *dest, int level);
API extern int File2Sha1(const char*,char*);


static void UploadCmdSingle(const s8*);

static u32 g_dwdstNode;
static u32 g_dwGuiNode;
static bool        g_bConnectedFlag;    
static bool        g_bSignFlag;         

static s16 wGuiAck;

static CCApp g_cCApp;

static u16 g_wTestSingleAppId;


#if _LINUX_
struct list_head tFileList;   
typedef struct tagFileList{
        struct list_head       tListHead;
        u8                     FileName[MAX_FILE_NAME_LENGTH];
        EM_FILE_STATUS         FileStatus;
        u16                    DealInstance;
        u32                    UploadFileSize;
        u32                    FileSize;
}TFileList;
#else
list<TFileList*> tFileList;
#endif

typedef struct tagDemoInfo{
        u32                    srcid;
        u8                     FileName[MAX_FILE_NAME_LENGTH];
        u32                    UploadFileSize;
        u32                    FileSize;
        EM_FILE_STATUS         emFileStatus;
}TDemoInfo;


typedef struct tagUploadAck{
        EM_FILE_STATUS emFileStatus;
        s16            wClientAck;
}TUploadAck;

typedef struct tagRemoveAck{
        bool           stableFlag;
        s16            wClientAck;
}TRemoveAck;

static TGuiAck tGuiAck;
static bool CheckFileIn(LPCSTR filename,TFileList **tFile);
static CCInstance* GetPendingIns();

void ShowApp(){

        u16 i;
        CCInstance *pIns;
        const char* state[] = {"IDLE","RUNNING"};


        OspAppShow();
     

        OspLog(SYS_LOG_LEVEL,"app  %d  ins info:\n",CLIENT_APP_ID);
        for(i = 1;i < MAX_INS_NUM;i++){

                pIns = (CCInstance*)((CApp*)&g_cCApp)->GetInstance(i);
                if(!pIns){
                        OspLog(LOG_LVL_ERROR,"[ShowApp]get error ins\n");
                        continue;
                }
                OspLog(SYS_LOG_LEVEL,"insid :  %d    state :  %s \n",i,state[pIns->m_curState]);
        }
}

void ShowInst(){

        OspInstShow(CLIENT_APP_ID);
}

void ShowRunInst(){

        u16 i;
        CCInstance *pIns;

        OspLog(SYS_LOG_LEVEL,"app  %d running ins info:\n",CLIENT_APP_ID);
        for(i = 1;i < MAX_INS_NUM;i++){

                pIns = (CCInstance*)((CApp*)&g_cCApp)->GetInstance(i);
                if(!pIns){
                        OspLog(LOG_LVL_ERROR,"[ShowApp]get error ins\n");
                        continue;
                }
                if(pIns->m_curState == RUNNING_STATE){
   ;
                }

        }

}

int clientInit(u32 guiPort){

        s16 i,j;

        g_bConnectedFlag = false;
        g_bSignFlag      = false;

#if _LINUX_
        INIT_LIST_HEAD(&tFileList);
#endif

        if(OSP_OK != g_cCApp.CreateApp("OspClientApp",CLIENT_APP_ID,CLIENT_APP_PRI,MAX_MSG_WAITING)){
                OspLog(LOG_LVL_ERROR,"[clientInit]app create error\n");
                return -1;
        }
#ifdef _LINUX_
        OspRegCommand("ShowRunInst",(void*)ShowRunInst,"");
        OspRegCommand("ShowInst",(void*)ShowInst,"");
        OspRegCommand("ShowApp",(void*)ShowApp,"");
        OspRegCommand("tcancel",(void*)Test_Cancel,"");
        OspRegCommand("tdisconnect",(void*)Test_DisConnect,"");
        OspRegCommand("Connect",(void*)Connect2Server,"");
        OspRegCommand("DisConnect",(void*)Disconnect2Server,"");
        OspRegCommand("SignIn",(void*)SendSignInCmd,"");
        OspRegCommand("SignOut",(void*)SendSignOutCmd,"");
        OspRegCommand("FileUpload",(void*)SendFileUploadCmd,"");
        OspRegCommand("MFileUpload",(void*)MultSendFileUploadCmd,"");
        OspRegCommand("Cancel",(void*)SendCancelCmd,"");
        OspRegCommand("Remove",(void*)SendRemoveCmd,"");
        OspRegCommand("GoOn",(void*)SendFileGoOnCmd,"");
#endif
  
        g_dwdstNode = OspConnectTcpNode(inet_addr(SERVER_IP),SERVER_PORT,10,3);
        if(INVALID_NODE == g_dwdstNode){
                OspLog(SYS_LOG_LEVEL, "[clientInit]Connect to server faild.\n");
        }else{
                OspLog(SYS_LOG_LEVEL, "[clientInit]Connect to server successfully.\n");
    
                if(OSP_OK !=OspNodeDiscCBReg(g_dwdstNode,CLIENT_APP_ID,CInstance::DAEMON)){
                    OspLog(LOG_LVL_ERROR,"[clientInit]regis disconnect error\n");
                    return -1;
                }
                g_bConnectedFlag = true;
        }

        g_dwGuiNode = OspConnectTcpNode(inet_addr(NATIVE_IP),guiPort,10,3);
        if(INVALID_NODE == g_dwGuiNode){
                OspLog(LOG_LVL_ERROR,"[clientInit]connect to native gui node failed\n");
                return -1;
        }

        return 0;
}


API void Test_Cancel(){

        Connect2Server();
        OspDelay(500);
        SendSignInCmd();
        OspDelay(500);
        SendFileUploadCmd();
        SendCancelCmd();
}

API void Test_DisConnect(){

        Connect2Server();
        OspDelay(500);
        SendSignInCmd();
        OspDelay(500);
        SendFileUploadCmd();
        OspDelay(9500);
        Disconnect2Server();
        OspDelay(500);
        Connect2Server();
        OspDelay(500);
        SendSignInCmd();
        OspDelay(500);
        SendFileUploadCmd();
}

API void Test_Sign(){

#if 0
        assert(0 == SendSignInCmd());
        assert(0 == SendSignOutCmd());

        SendSignInCmd();
        SendSignInCmd();
#endif
}

API void Connect2Server(){

        u16 i;

        g_dwdstNode = OspConnectTcpNode(inet_addr(SERVER_IP),SERVER_PORT,10,3);
        if(INVALID_NODE == g_dwdstNode){
                OspLog(LOG_LVL_KEY, "Connect extern node failed. exit.\n");
                return;
        }

        if(OSP_OK !=OspNodeDiscCBReg(g_dwdstNode,CLIENT_APP_ID,CInstance::DAEMON)){
            OspLog(LOG_LVL_ERROR,"[main]regis disconnect error\n");
            return;
        }

        g_bConnectedFlag = true;
}

API void Disconnect2Server(){

        if(!OspDisconnectTcpNode(g_dwdstNode)){
                OspLog(LOG_LVL_ERROR,"[Disconnect2Server]disconnecte failed\n");
                return;
        }
}

API void SendFileGoOnCmd(){

       s8 file_name[] = MY_FILE_NAME;

       if(OSP_OK != ::OspPost(MAKEIID(CLIENT_APP_ID,CInstance::DAEMON),FILE_GO_ON_CMD,
                               file_name,strlen(file_name)+1)){
               OspLog(LOG_LVL_ERROR,"[SendCancel] post error\n");
       }

}

API void SendCancelCmd(){

       s8 file_name[] = MY_FILE_NAME;

       if(OSP_OK != ::OspPost(MAKEIID(CLIENT_APP_ID,CInstance::DAEMON),SEND_CANCEL_CMD,
                               file_name,strlen(file_name)+1)){
               OspLog(LOG_LVL_ERROR,"[SendCancel] post error\n");
       }
}

API void SendRemoveCmd(){

       s8 file_name[] = MY_FILE_NAME;

       if(OSP_OK != ::OspPost(MAKEIID(CLIENT_APP_ID,CInstance::DAEMON),SEND_REMOVE_CMD,
                       file_name,strlen(file_name)+1)){
              OspLog(LOG_LVL_ERROR,"[SendRemoveCmd] post error\n");
       }
}

API void SendSignInCmd(){

        TSinInfo tSinInfo;

        strcpy((LPSTR)tSinInfo.Username,"Robert");
        strcpy((LPSTR)tSinInfo.Passwd,"admin");

        if(OSP_OK != ::OspPost(MAKEIID(CLIENT_APP_ID,CInstance::DAEMON),SIGN_IN_CMD,
                        &tSinInfo,sizeof(tSinInfo))){
               OspLog(LOG_LVL_ERROR,"[SendSignInCmd] post error\n");
               return;
        }

}

API void SendSignOutCmd(){

        if(OSP_OK != ::OspPost(MAKEIID(CLIENT_APP_ID,CInstance::DAEMON),SIGN_OUT_CMD)){
               OspLog(LOG_LVL_ERROR,"[SendSignOutCmd] post error\n");
        }
}


static void UploadCmdSingle(const s8* filename){

        if(OSP_OK !=::OspPost(MAKEIID(CLIENT_APP_ID,CInstance::DAEMON),FILE_UPLOAD_CMD,
                        filename,strlen(filename)+1)){
               OspLog(LOG_LVL_ERROR,"[UploadCmdSingle] post error\n");
        }
}

API void SendFileUploadCmd(){

        UploadCmdSingle(MY_FILE_NAME);
}

API void MultSendFileUploadCmd(){

        UploadCmdSingle(MY_FILE_NAME);
        OspDelay(200);
        UploadCmdSingle("test_file_name");
}

void CCInstance::FileUploadCmd(CMessage*const pMsg){

        char sha1Buffer[41];

        if(!g_bSignFlag){
                OspLog(SYS_LOG_LEVEL,"[FileUploadCmd]did not sign in\n");
                return;
        }

        if(!pMsg->content || pMsg->length <= 0){
                 OspLog(LOG_LVL_ERROR,"[FileUploadCmd] pMsg is NULL\n");
                 return;
        }

        if(pMsg->length > MAX_FILE_NAME_LENGTH){
                OspLog(SYS_LOG_LEVEL,"[FileUploadCmd]file is not finished or removed\n");
                wGuiAck = -10;
                goto post2gui;
        }


        if(File2Sha1((const char*)pMsg->content,sha1Buffer) != 0){
                OspLog(LOG_LVL_ERROR,"[FileUploadCmd]file:%s sha1 error\n",pMsg->content);
                wGuiAck = -11;
                goto post2gui;
        }else{
                OspLog(SYS_LOG_LEVEL,"[FileUploadCmd]file:%s sha1:%s\n",pMsg->content,sha1Buffer);
        }

        if(OSP_OK != post(MAKEIID(SERVER_APP_ID,CInstance::DAEMON),FILE_SHA1
                       ,sha1Buffer,41*sizeof(char),g_dwdstNode)){
                OspLog(LOG_LVL_ERROR,"[FileUploadCmdDeal] post error\n");
                return;
        }


}
#if 0
void CCInstance::FileUploadCmd(CMessage*const pMsg){

        CCInstance *ccIns;
        TFileList *tnFile = NULL;
        FILE * zFile,*zFileTemp;
        char tempFileName[MAX_FILE_NAME_LENGTH];
        char sha1Buffer[41];



        wGuiAck = 0;



        if(!g_bSignFlag){
                OspLog(SYS_LOG_LEVEL,"[FileUploadCmd]did not sign in\n");
                return;
        }

        if(!pMsg->content || pMsg->length <= 0){
                 OspLog(LOG_LVL_ERROR,"[FileUploadCmd] pMsg is NULL\n");
                 return;
        }

        if(pMsg->length > MAX_FILE_NAME_LENGTH){
                OspLog(SYS_LOG_LEVEL,"[FileUploadCmd]file is not finished or removed\n");
                wGuiAck = -10;
                goto post2gui;
        }


        if(File2Sha1((const char*)pMsg->content,sha1Buffer) != 0){
                OspLog(LOG_LVL_ERROR,"[FileUploadCmd]file:%s sha1 error\n",pMsg->content);
                wGuiAck = -11;
                goto post2gui;
        }else{
                OspLog(SYS_LOG_LEVEL,"[FileUploadCmd]file:%s sha1:%s\n",pMsg->content,sha1Buffer);
        }

#if 0

        strcpy(tempFileName,(LPCSTR)pMsg->content);
        strcat(tempFileName,"_temp");
        if(!(zFile = fopen((LPCSTR)pMsg->content,"rb"))){
                 OspLog(LOG_LVL_ERROR,"[FileUploadCmd] open zfile error\n");
                 wGuiAck = -30;
                 goto post2gui;
        }

       if(!(zFileTemp = fopen((LPCSTR)tempFileName,"wb"))){
                 OspLog(LOG_LVL_ERROR,"[FileUploadCmd]open zfiletemp:%s error\n",tempFileName);
                 wGuiAck = -31;
                 goto post2gui;
        }
        if((def(zFile,zFileTemp,Z_DEFAULT_COMPRESSION)) != Z_OK){
                 OspLog(LOG_LVL_ERROR,"[FileUploadCmd] def error\n");
                 wGuiAck = -32;
                         //TODO:
                 goto post2gui;
        }
        fclose(zFile);
        fclose(zFileTemp);
#endif

        if(CheckFileIn((LPCSTR)pMsg->content,&tnFile) && STATUS_FINISHED != tnFile->FileStatus
                        && STATUS_REMOVED != tnFile->FileStatus){
                OspLog(SYS_LOG_LEVEL,"[FileUploadCmd]file is not finished or removed\n");
                wGuiAck = -12;
                goto post2gui;
       }

        if(!(ccIns = GetPendingIns())){
                OspLog(SYS_LOG_LEVEL, "[FileUploadCmd]no pending instance,please wait...\n");
                wGuiAck = -13;
                goto post2gui;
        }

        if(OSP_OK != post(MAKEIID(CLIENT_APP_ID,ccIns->GetInsID()),FILE_UPLOAD_CMD_DEAL,
                        pMsg->content,pMsg->length)){
               OspLog(LOG_LVL_ERROR,"[FileUploadCmd] post error\n");
               wGuiAck = -14;
               goto post2gui;
        }
        if(!tnFile){
            tnFile = new TFileList();
            if(!tnFile){
                OspLog(LOG_LVL_ERROR,"[FileUploadCmd]file list item malloc failed\n");
                wGuiAck = -15;
                goto post2gui;
            }
#if _LINUX_
            list_add(&tnFile->tListHead,&tFileList);
#else
            tFileList.push_back(tnFile);
#endif
            strcpy((LPSTR)tnFile->FileName,(LPSTR)pMsg->content);

        }

        ccIns->m_curState = RUNNING_STATE;
        tnFile->DealInstance = ccIns->GetInsID();
        tnFile->FileStatus = STATUS_UPLOADING;

post2gui:
        tGuiAck.wGuiAck = wGuiAck;
        strcpy((LPSTR)tGuiAck.FileName,(LPCSTR)pMsg->content);

        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_FILE_UPLOAD_ACK
               ,&tGuiAck,sizeof(tGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[FileUploadCmd]post error\n");
        }
        return;
}
#endif

void CCInstance::FileUploadCmdDeal(CMessage *const pMsg){

        struct list_head *tFileHead,*templist;
        TFileList *tnFile;


        if(!g_bSignFlag){
                OspLog(SYS_LOG_LEVEL,"[FileUploadCmdDeal]did not sign in\n");
                return;
        }
#if 0
        if(!pMsg->content || pMsg->length <= 0){
                 OspLog(LOG_LVL_ERROR,"[FileUploadCmdDeal] pMsg is NULL\n");
                 printf("[FileUploadCmdDeal]msg is null\n");
                 return;
        }

#endif
        wGuiAck = 0;
        m_dwUploadFileSize = 0;
        strcpy((LPSTR)file_name_path,(LPCSTR)pMsg->content);
        strcpy((LPSTR)tGuiAck.FileName,(LPCSTR)pMsg->content);
        OspLog(LOG_LVL_ERROR,"[FileUploadCmdDeal] file :%s\n",file_name_path);
        
        if(!(file = fopen((LPCSTR)file_name_path,"rb"))){
		
                OspLog(LOG_LVL_ERROR,"[FileUploadCmdDeal]open file failed\n");
                wGuiAck = -17;
                goto postError2gui;
        }
  	
        if(fseek(file,0L,SEEK_END) != 0){
                 OspLog(LOG_LVL_ERROR,"[FileUploadCmdDeal] file fseeek error\n");
                 wGuiAck = -18;
                 fclose(file);
                 file = NULL;
                 goto postError2gui;
        }
        if(-1L == (m_dwFileSize = ftell(file))){
                 OspLog(LOG_LVL_ERROR,"[FileUploadCmdDeal] file ftell error\n");
                 wGuiAck = -19;
                 fclose(file);
                 file = NULL;
                 goto postError2gui;
        }

        rewind(file);

        if(OSP_OK != post(MAKEIID(SERVER_APP_ID,CInstance::DAEMON),FILE_SEND_UPLOAD
                       ,pMsg->content,pMsg->length,g_dwdstNode)){
                OspLog(LOG_LVL_ERROR,"[FileUploadCmdDeal] post error\n");
                return;
        }

        emFileStatus = STATUS_SEND_UPLOAD;

        tGuiAck.dwFileSize = m_dwFileSize;
        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_FILE_SIZE_ACK
               ,&tGuiAck,sizeof(tGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[FileUploadCmdDeal]post error\n");
        }

        OspLog(SYS_LOG_LEVEL,"[FileUploadCmdDeal]send upload\n");
        return;
   

postError2gui:
        CheckFileIn((LPCSTR)file_name_path,&tnFile);
        delete tnFile;
        NextState(IDLE_STATE);

        tGuiAck.wGuiAck = wGuiAck;
        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_FILE_UPLOAD_ACK
               ,&tGuiAck,sizeof(tGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[FileUploadCmdDeal]post error\n");
        }
        return;
}

void CCInstance::FileUploadAck(CMessage* const pMsg){

        size_t buffer_size;
        struct list_head *tFileHead,*templist;
        TFileList *tnFile;
        TUploadAck *tUploadAck;

        wGuiAck = 0;

        if(!g_bSignFlag){
                OspLog(LOG_LVL_ERROR,"[FileUploadAck]sign out\n");
                return;
        }

        if(!pMsg->content || pMsg->length <= 0){
                 OspLog(LOG_LVL_ERROR,"[FileUploadAck] pMsg content is NULL\n");
                 wGuiAck = -22;
                 goto postError2gui;
        }

        m_dwDisInsID = pMsg->srcid;
        tUploadAck = (TUploadAck*)pMsg->content;
        emFileStatus = tUploadAck->emFileStatus;

        strcpy((LPSTR)tGuiAck.FileName,(LPCSTR)file_name_path);
        if(0 != tUploadAck->wClientAck){
                wGuiAck = tUploadAck->wClientAck;
                goto postError2gui;
        }
        if(emFileStatus == STATUS_UPLOADING){
                buffer_size = fread(buffer,1,sizeof(s8)*BUFFER_SIZE,file);
                if(ferror(file)){
                        OspLog(LOG_LVL_ERROR,"[FileUploadAck] read-file error\n");
                        wGuiAck = -23;
                        goto postError2gui;
                }
                if(feof(file)){
                     if(OSP_OK != post(pMsg->srcid,FILE_FINISH
                                   ,buffer,buffer_size,g_dwdstNode)){
                          OspLog(LOG_LVL_ERROR,"[FileUploadAck]FILE_FINISH post error\n");
                     }
           
                }else{
                     if(OSP_OK != post(pMsg->srcid,FILE_UPLOAD
                                   ,buffer,buffer_size,g_dwdstNode)){
                          OspLog(LOG_LVL_ERROR,"[FileUploadAck]FILE_UPLOAD post error\n");
                          return;
                     }
                }
                m_dwUploadFileSize += buffer_size;

                tGuiAck.dwUploadFileSize = m_dwUploadFileSize;
                if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_UPLOAD_FILE_SIZE_ACK
                       ,&tGuiAck,sizeof(tGuiAck),g_dwGuiNode)){
                        OspLog(LOG_LVL_ERROR,"[FileUploadAck]post error\n");
                }
        }else if(emFileStatus == STATUS_RECEIVE_CANCEL){
                if(OSP_OK != post(pMsg->srcid,FILE_CANCEL
                              ,NULL,0,g_dwdstNode)){
                     OspLog(LOG_LVL_ERROR,"[FileUploadAck]FILE_CANCEL post error\n");
                     return;
                }
        }else if(emFileStatus == STATUS_RECEIVE_REMOVE){
                if(OSP_OK != post(pMsg->srcid,FILE_REMOVE
                              ,NULL,0,g_dwdstNode)){
                     OspLog(LOG_LVL_ERROR,"[FileUploadAck]FILE_REMOVE post error\n");
                     return;
                }
        }else{
                OspLog(LOG_LVL_ERROR,"[FileUploadAck]get incorrect file status:%d\n",emFileStatus);
                printf("[FileUploadAck]get incorrect file status\n");
                wGuiAck = -24;
                goto postError2gui;
        }

        return;

postError2gui:


#if _MSC_VER
#if 0
        auto p = tFileList.begin();
        while(p != tFileList.end()){
                if(strcmp((LPCSTR)p->FileName,(LPCSTR)file_name_path) == 0)
                tFileList.erase(p);
//                delete (char*)p;
                break;
        }
#endif
        CheckFileIn((LPCSTR)file_name_path,&tnFile);
        delete tnFile;

#else

        CheckFileIn((LPCSTR)file_name_path,&tnFile);
        delete tnFile;
#endif

        NextState(IDLE_STATE);
        fclose(file);
        file = NULL;

        tGuiAck.wGuiAck = wGuiAck;
        strcpy((LPSTR)tGuiAck.FileName,(LPCSTR)file_name_path);
        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_FILE_UPLOAD_ACK
               ,&tGuiAck,sizeof(tGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[FileUploadCmdDeal]post error\n");
        }
        return;
}

void CCInstance::FileFinishAck(CMessage* const pMsg){
        
        TFileList *tFile;

        wGuiAck = 0;

        if(!g_bSignFlag){
                OspLog(SYS_LOG_LEVEL,"[FileFinishAck]did not sign in\n");
                return;
        }
        if(!pMsg->content || pMsg->length <= 0){
                wGuiAck = -1;
        }else{
                wGuiAck = *(u16*)pMsg->content;
        }
        if(fclose(file) == 0){
                OspLog(SYS_LOG_LEVEL,"[FileFinishAck]file closed\n");

        }else{
                OspLog(LOG_LVL_ERROR,"[FileFinishAck]file close failed\n");
        }
        file = NULL;

        CheckFileIn((LPCSTR)file_name_path,&tFile);
        tFile->FileStatus = STATUS_FINISHED;

        OspLog(SYS_LOG_LEVEL,"[FileFinishAck]file upload finish\n");
        emFileStatus = STATUS_FINISHED;
        NextState(IDLE_STATE);

        tGuiAck.wGuiAck = wGuiAck;
        strcpy((LPSTR)tGuiAck.FileName,(LPCSTR)file_name_path);
        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_FILE_FINISHED_ACK
               ,&tGuiAck,sizeof(tGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[FileUploadCmdDeal]post error\n");
        }
        return;
#if 0
        m_dwFileSize = 0;
        m_dwUploadFileSize = 0;
#endif
}

void CCInstance::SignInCmd(CMessage *const pMsg){

        wGuiAck = 0;

        if(!g_bConnectedFlag){

#if 0
                scanf("ip:%s port:%d\n",server_ip,server_port);
                g_dwdstNode = OspConnectTcpNode(inet_addr(SERVER_IP),SERVER_PORT,10,3);
                if(INVALID_NODE == g_dwdstNode){
                        OspLog(LOG_LVL_ERROR, "[SignInCmd]Connect to server faild.\n");
                        return;
                }else{
                        m_bConnectedFlag = true;
                }
#endif
                OspLog(SYS_LOG_LEVEL,"[SignInCmd]not connected\n");
                return;
        }

        if(!pMsg->content || pMsg->length <= 0){
                OspLog(LOG_LVL_ERROR,"[SignInCmd] pMsg content is NULL\n");
                return;
        }
        if(g_bSignFlag){
                OspLog(SYS_LOG_LEVEL,"[SignInCmd]sign in already\n");
                wGuiAck = -15;
                goto post2gui;
        }

        if(post(MAKEIID(SERVER_APP_ID,CInstance::DAEMON),SIGN_IN
                    ,pMsg->content,pMsg->length,g_dwdstNode) != OSP_OK){
                OspLog(LOG_LVL_ERROR,"[SignInCmd] post error\n");
                return;
        }
        return;
post2gui:
        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_SIGN_IN_ACK
               ,&wGuiAck,sizeof(wGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[SignInCmd]post error\n");
        }
        return;
}

void CCInstance::SignInAck(CMessage * const pMsg){

        wGuiAck = 0;

        if(!g_bConnectedFlag){
                OspLog(LOG_LVL_ERROR,"[SignInAck]disconnected\n");
                return;
        }

        if(pMsg->length <= 0 || !pMsg->content){
                OspLog(SYS_LOG_LEVEL,"[SignInAck]sign in failed\n");
                wGuiAck = -12;
                goto post2gui;
        }

        wGuiAck = *(s16*)(pMsg->content);
        if(0 == wGuiAck){
                g_bSignFlag = true;
                OspLog(SYS_LOG_LEVEL,"[SignInAck]sign in successfully\n");
        }

post2gui:
        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_SIGN_IN_ACK
               ,&wGuiAck,sizeof(wGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[SignInAck]post error\n");
        }
        return;
}

void CCInstance::SignOutCmd(CMessage * const pMsg){

        TFileList *tnFile;
        struct list_head *tFileHead,*templist;
        CCInstance *pIns;

        wGuiAck = 0;
#if 0
        if(!m_bConnectedFlag){
                OspLog(LOG_LVL_ERROR,"[SignOutCmd]disconnected\n");
                return;
        }
#endif

        if(!g_bSignFlag){
                OspLog(SYS_LOG_LEVEL,"[SignOutCmd]haven't sign in\n");
                return;
        }

#if 0
        if(emFileStatus >= STATUS_SEND_UPLOAD&&
                        emFileStatus <= STATUS_RECEIVE_REMOVE){
                if(OSP_OK != post(MAKEIID(GetAppID(),GetInsID()),SIGN_OUT_CMD
                               ,NULL,0)){
                        OspLog(LOG_LVL_ERROR,"[SignOutCmd] post error\n");
                }
                return;
        }
#endif

#if _LINUX_
      
        list_for_each_safe(tFileHead,templist,&tFileList){
                tnFile = list_entry(tFileHead,TFileList,tListHead);
                pIns = (CCInstance*)((CApp*)&g_cCApp)->GetInstance(tnFile->DealInstance);
                if(!pIns){
                        OspLog(LOG_LVL_ERROR,"[SignOutAck]get error ins\n");
                        continue;
                }
                
                if(tnFile->FileStatus == STATUS_UPLOADING){
                        tnFile->FileStatus = STATUS_CANCELLED;
                        tnFile->UploadFileSize = pIns->m_dwUploadFileSize;
                        tnFile->FileSize = pIns->m_dwFileSize;
//                                pIns->emFileStatus = STATUS_CANCELLED;
                        pIns->m_curState = IDLE_STATE;
                        if(pIns->file){
                                if(fclose(pIns->file) == 0){
                                        OspLog(SYS_LOG_LEVEL,"[SignOutAck]file closed\n");
                                }else{
                                        OspLog(LOG_LVL_ERROR,"[SignOutAck]file close failed\n");
                                }
                                file = NULL;
                        }
                        continue;
                }

                if(tnFile->FileStatus >= STATUS_CANCELLED)
                        continue;

                tnFile->FileStatus = STATUS_INIT;
                pIns->m_curState = IDLE_STATE;
                pIns->emFileStatus = STATUS_INIT;
                if(pIns->file){
                        if(fclose(pIns->file) == 0){
                                OspLog(SYS_LOG_LEVEL,"[SignOutAck]file closed\n");
                        }else{
                                OspLog(LOG_LVL_ERROR,"[SignOutAck]file close failed\n");
                        }
                        pIns->file = NULL;
                }
        }
#else
        list<TFileList*>::iterator iter = tFileList.begin();
        while(iter != tFileList.end()){

                pIns = (CCInstance*)((CApp*)&g_cCApp)->GetInstance((*iter)->DealInstance);
                if(!pIns){
                        OspLog(LOG_LVL_ERROR,"[SignOutAck]get error ins\n");
                        continue;
                }
                
                if((*iter)->FileStatus == STATUS_UPLOADING){
                        (*iter)->FileStatus = STATUS_CANCELLED;
                        (*iter)->UploadFileSize = pIns->m_dwUploadFileSize;
                        (*iter)->FileSize = pIns->m_dwFileSize;

                        pIns->m_curState = IDLE_STATE;
                        if(pIns->file){
                                if(fclose(pIns->file) == 0){
                                        OspLog(SYS_LOG_LEVEL,"[SignOutAck]file closed\n");
                                }else{
                                        OspLog(LOG_LVL_ERROR,"[SignOutAck]file close failed\n");
                                }
                                file = NULL;
                        }
                        continue;
                }

                if((*iter)->FileStatus >= STATUS_CANCELLED)
                        continue;

                (*iter)->FileStatus = STATUS_INIT;
                pIns->m_curState = IDLE_STATE;
                pIns->emFileStatus = STATUS_INIT;
                if(pIns->file){
                        if(fclose(pIns->file) == 0){
                                OspLog(SYS_LOG_LEVEL,"[SignOutAck]file closed\n");
                        }else{
                                OspLog(LOG_LVL_ERROR,"[SignOutAck]file close failed\n");
                        }
                        pIns->file = NULL;
                }
                iter++;
        }
#endif

        if(OSP_OK != post(MAKEIID(SERVER_APP_ID,CInstance::DAEMON)
                                ,SIGN_OUT,NULL,0,g_dwdstNode)){
                OspLog(LOG_LVL_ERROR,"[SignOutCmd] post error\n");
                return;
        }
        if(0 == wGuiAck){
                OspLog(SYS_LOG_LEVEL,"get sign out cmd,send to server\n");
        }
        return;
}

void CCInstance::SignOutAck(CMessage * const pMsg){

        wGuiAck = 0;

        if(!g_bConnectedFlag){
                OspLog(LOG_LVL_ERROR,"[SignOutAck]disconnected\n");
                return;
        }

        if(pMsg->length <= 0 || !pMsg->content){
                OspLog(SYS_LOG_LEVEL,"[SignOutAck]sign in failed\n");
                wGuiAck = -12;
                goto post2gui;
        }

        wGuiAck = *(u16*)(pMsg->content);
        if(0 == wGuiAck){
                g_bSignFlag = false;
                OspLog(SYS_LOG_LEVEL,"[SignOutAck]sign out\n");

        }
        
post2gui:
        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_SIGN_OUT_ACK
               ,&wGuiAck,sizeof(wGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[SignOutAck]post error\n");
        }
        return;
}

void CCInstance::MsgProcessInit(){

    
        RegMsgProFun(MAKEESTATE(IDLE_STATE,SIGN_IN_CMD),&CCInstance::SignInCmd,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,SIGN_IN_ACK),&CCInstance::SignInAck,&m_tCmdDaemonChain);

        RegMsgProFun(MAKEESTATE(IDLE_STATE,SIGN_OUT_CMD),&CCInstance::SignOutCmd,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,SIGN_OUT_ACK),&CCInstance::SignOutAck,&m_tCmdDaemonChain);

        
        RegMsgProFun(MAKEESTATE(IDLE_STATE,FILE_UPLOAD_CMD),&CCInstance::FileUploadCmd,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,SEND_CANCEL_CMD),&CCInstance::CancelCmd,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,FILE_GO_ON_CMD),&CCInstance::FileGoOnCmd,&m_tCmdDaemonChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,SEND_REMOVE_CMD),&CCInstance::RemoveCmd,&m_tCmdDaemonChain);


        RegMsgProFun(MAKEESTATE(RUNNING_STATE,FILE_UPLOAD_ACK),&CCInstance::FileUploadAck,&m_tCmdChain);
        RegMsgProFun(MAKEESTATE(RUNNING_STATE,FILE_FINISH_ACK),&CCInstance::FileFinishAck,&m_tCmdChain);
        RegMsgProFun(MAKEESTATE(RUNNING_STATE,FILE_CANCEL_ACK),&CCInstance::FileCancelAck,&m_tCmdChain);
        RegMsgProFun(MAKEESTATE(RUNNING_STATE,FILE_REMOVE_ACK),&CCInstance::FileRemoveAck,&m_tCmdChain);
     

        RegMsgProFun(MAKEESTATE(RUNNING_STATE,FILE_UPLOAD_CMD_DEAL),&CCInstance::FileUploadCmdDeal,&m_tCmdChain);
        RegMsgProFun(MAKEESTATE(RUNNING_STATE,SEND_REMOVE_CMD_DEAL),&CCInstance::RemoveCmdDeal,&m_tCmdChain);
        RegMsgProFun(MAKEESTATE(RUNNING_STATE,SEND_CANCEL_CMD_DEAL),&CCInstance::CancelCmdDeal,&m_tCmdChain);
        RegMsgProFun(MAKEESTATE(RUNNING_STATE,SEND_STABLE_REMOVE_CMD_DEAL),&CCInstance::StableRemoveCmdDeal,
                        &m_tCmdChain);
        RegMsgProFun(MAKEESTATE(RUNNING_STATE,FILE_GO_ON_CMD_DEAL),&CCInstance::FileGoOnCmdDeal,&m_tCmdChain);


        RegMsgProFun(MAKEESTATE(IDLE_STATE,SEND_CANCEL_CMD),&CCInstance::CancelCmd,&m_tCmdChain);
        RegMsgProFun(MAKEESTATE(IDLE_STATE,FILE_GO_ON_CMD),&CCInstance::FileGoOnCmd,&m_tCmdChain);

}

void CCInstance::NodeChainEnd(){

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

bool CCInstance::RegMsgProFun(u32 EventState,MsgProcess c_MsgProcess,tCmdNode** tppNodeChain){

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

bool CCInstance::FindProcess(u32 EventState,MsgProcess* c_MsgProcess,tCmdNode* tNodeChain){

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

void CCInstance::InstanceEntry(CMessage * const pMsg){

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

void CCInstance::DaemonInstanceEntry(CMessage *const pMsg,CApp *pCApp){

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
        }
}


void CCInstance::RemoveCmd(CMessage* const pMsg){

    TFileList *tFile;
	CCInstance *ccIns;

    wGuiAck = 0;
    if(!g_bSignFlag){
            OspLog(SYS_LOG_LEVEL,"[RemoveCmd]not sign in\n");
            return;
    }

    if(!pMsg->content || pMsg->length <= 0){
             OspLog(LOG_LVL_ERROR,"[RemoveCmd]Msg is NULL\n");
             return;
    }

    if(!CheckFileIn((LPCSTR)pMsg->content,&tFile)){
            OspLog(LOG_LVL_ERROR,"[RemoveCmd]file not in list\n");
            wGuiAck = -12;
            goto postError2gui;
    }

#if 0
 
    if(tFile->FileStatus >= STATUS_UPLOAD_CMD
                    && tFile->FileStatus <= STATUS_RECEIVE_REMOVE){
                if(OSP_OK != post(MAKEIID(GetAppID(),GetInsID()),SEND_REMOVE_CMD
                               ,pMsg->content,pMsg->length)){
                        OspLog(LOG_LVL_ERROR,"[RemoveCmd] post error\n");
                }
                return;

    }
#endif
    if(tFile->FileStatus == STATUS_UPLOADING){
            if(OSP_OK != post(MAKEIID(CLIENT_APP_ID,tFile->DealInstance),SEND_REMOVE_CMD_DEAL
                           ,pMsg->content,strlen((LPCSTR)pMsg->content)+1)){
                    OspLog(LOG_LVL_ERROR,"[RemoveCmd] post error\n");
                    return;
            }
            tFile->FileStatus = STATUS_REMOVE_CMD;
            return;
    }

    if(tFile->FileStatus == STATUS_REMOVED){
            OspLog(SYS_LOG_LEVEL, "[RemoveCmd]file already removed\n");
            wGuiAck = -14;
            goto postError2gui;
    }

    if(!(ccIns = GetPendingIns())){
            OspLog(SYS_LOG_LEVEL, "[RemoveCmd]no pending instance,please wait...\n");
            wGuiAck = -15;
            goto postError2gui;
    }

    if(OSP_OK != post(MAKEIID(CLIENT_APP_ID,ccIns->GetInsID()),SEND_STABLE_REMOVE_CMD_DEAL
                   ,pMsg->content,strlen((LPCSTR)pMsg->content)+1)){
            OspLog(LOG_LVL_ERROR,"[RemoveCmd] post error\n");
            return;
    }
    ccIns->m_curState = RUNNING_STATE;
    tFile->FileStatus = STATUS_REMOVE_CMD;
    tFile->DealInstance = ccIns->GetInsID();
    return;

postError2gui:

        tGuiAck.wGuiAck = wGuiAck;
        strcpy((LPSTR)tGuiAck.FileName,(LPCSTR)pMsg->content);
        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_FILE_CANCEL_ACK
               ,&tGuiAck,sizeof(tGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[RemoveCmd]post error\n");
        }
        return;
}

void CCInstance::RemoveCmdDeal(CMessage* const pMsg){ 

    if(!g_bSignFlag){
            OspLog(SYS_LOG_LEVEL,"[RemoveCmdDeal]did not sign in\n");
            return;
    }

    if(OSP_OK != post(m_dwDisInsID,SEND_REMOVE
                   ,pMsg->content,pMsg->length,g_dwdstNode)){
            OspLog(LOG_LVL_ERROR,"[RemoveCmdDeal] post error\n");
            return;
    }
    emFileStatus = STATUS_SEND_REMOVE;

}

void CCInstance::StableRemoveCmdDeal(CMessage* const pMsg){ 

    if(!g_bSignFlag){
            OspLog(SYS_LOG_LEVEL,"[StableRemoveCmdDeal]did not sign in\n");
            return;
    }

    if(OSP_OK != post(MAKEIID(SERVER_APP_ID,DAEMON),FILE_STABLE_REMOVE
                   ,pMsg->content,pMsg->length,g_dwdstNode)){
            OspLog(LOG_LVL_ERROR,"[StableRemoveCmdDeal] post error\n");
            return;
    }


    strcpy((LPSTR)file_name_path,(LPCSTR)pMsg->content);
    emFileStatus = STATUS_SEND_REMOVE;

}

void CCInstance::FileRemoveAck(CMessage* const pMsg){

        TFileList *tFile;
        TRemoveAck *tRemoveAck;


        wGuiAck  = 0;

        if(!g_bSignFlag){
                OspLog(SYS_LOG_LEVEL,"[FileRemoveAck]did not sign in\n");
                return;
        }

        if(!pMsg->content || pMsg->length <= 0){
                wGuiAck = -1;
                goto post2gui;
        }

        tRemoveAck = (TRemoveAck*)pMsg->content;
        if(tRemoveAck->wClientAck != 0){
                wGuiAck = tRemoveAck->wClientAck;
                goto post2gui;
        }

        if(!tRemoveAck->stableFlag){
                if(fclose(file) == 0){
                        OspLog(SYS_LOG_LEVEL,"[FileRemoveAck]file closed\n");
                }else{
                        OspLog(LOG_LVL_ERROR,"[FileRemoveAck]file close failed\n");
                }
                file = NULL;
        }

        CheckFileIn((LPCSTR)file_name_path,&tFile);
        tFile->FileStatus = STATUS_REMOVED;
        emFileStatus = STATUS_REMOVED;
#if 0
        m_dwUploadFileSize = 0;
        m_dwFileSize = 0;
#endif
        NextState(IDLE_STATE);
  
        OspLog(SYS_LOG_LEVEL,"[FileRemoveAck]file removed\n");

post2gui:
        tGuiAck.wGuiAck = wGuiAck;
        strcpy((LPSTR)tGuiAck.FileName,(LPCSTR)file_name_path);
        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_FILE_REMOVE_ACK
               ,&tGuiAck,sizeof(tGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[FileRemoveAck]post error\n");
        }
        return;
}


void CCInstance::CancelCmd(CMessage* const pMsg){

        TFileList *tFile;

        wGuiAck = 0;

        if(!g_bSignFlag){
                OspLog(SYS_LOG_LEVEL,"[CancelCmd]did not sign in\n");
                return;
        }

        if(!pMsg->content || pMsg->length <= 0){
                 OspLog(LOG_LVL_ERROR,"[CancelCmd]Msg is NULL\n");
                 return;
        }

        if(!CheckFileIn((LPCSTR)pMsg->content,&tFile)){
                OspLog(LOG_LVL_ERROR,"[CancelCmd]file not in list\n");
                wGuiAck = -12;
                goto postError2gui;
        }

#if 0
   
        if(tFile->FileStatus >= STATUS_UPLOAD_CMD
                        && tFile->FileStatus <= STATUS_RECEIVE_REMOVE){
                    if(OSP_OK != post(MAKEIID(GetAppID(),GetInsID()),SEND_CANCEL_CMD
                                   ,pMsg->content,pMsg->length)){
                            OspLog(LOG_LVL_ERROR,"[CancelCmd] post error\n");
                    }
                    return;

        }
#endif

    
        if(tFile->FileStatus > STATUS_UPLOADING){
                OspLog(LOG_LVL_ERROR,"[CancelCmd]file already stable\n");
                wGuiAck = -13;
                goto postError2gui;
        }

        if(OSP_OK != post(MAKEIID(CLIENT_APP_ID,tFile->DealInstance),SEND_CANCEL_CMD_DEAL
                       ,pMsg->content,pMsg->length)){
                OspLog(LOG_LVL_ERROR,"[CancelCmd] post error\n");
                wGuiAck = -14;
                goto postError2gui;
        }
        tFile->FileStatus = STATUS_CANCEL_CMD;
        return;

postError2gui:

        tGuiAck.wGuiAck = wGuiAck;
        strcpy((LPSTR)tGuiAck.FileName,(LPCSTR)pMsg->content);
        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_FILE_CANCEL_ACK
               ,&wGuiAck,sizeof(wGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[CancelCmd]post error\n");
        }
        return;
}

void CCInstance::CancelCmdDeal(CMessage* const pMsg){

        if(!g_bSignFlag){
                OspLog(SYS_LOG_LEVEL,"[CancelCmdDeal]did not sign in\n");
                return;
        }

#if 0
     
        if(emFileStatus >= STATUS_SEND_UPLOAD&&
                        emFileStatus <= STATUS_RECEIVE_REMOVE){
                if(OSP_OK != post(MAKEIID(GetAppID(),GetInsID()),SEND_CANCEL_CMD_DEAL
                               ,NULL,0)){
                        OspLog(LOG_LVL_ERROR,"[CancelCmdDeal] post error\n");
                }
                return;
        }

#endif

        if(OSP_OK != post(m_dwDisInsID,SEND_CANCEL
                       ,pMsg->content,pMsg->length,g_dwdstNode)){
                OspLog(LOG_LVL_ERROR,"[CancelCmdDeal] post error\n");
                return;
        }

        OspLog(SYS_LOG_LEVEL,"[CancelCmdDeal]send cancel\n");
        emFileStatus = STATUS_SEND_CANCEL;
}

void CCInstance::FileCancelAck(CMessage* const pMsg){

        u16 wAppId;
        TFileList *tFile;
        u16 *wClientAck;

        wGuiAck = 0;
        NextState(IDLE_STATE);

        if(!g_bSignFlag){
                OspLog(SYS_LOG_LEVEL,"[FileCancelAck]not sign in\n");
                return;
        }
        if(!pMsg->content || pMsg->length <= 0){
                OspLog(LOG_LVL_ERROR,"[FileCancelAck]file close failed\n");
                wGuiAck = -1;
                goto post2gui;
        }

        wClientAck = (u16*)pMsg->content;
        if((wGuiAck = *wClientAck) != 0){
                goto post2gui;
        }
        if(fclose(file) == 0){
                OspLog(SYS_LOG_LEVEL,"[FileCancelAck]file closed\n");
                file = NULL;
        }else{
                OspLog(LOG_LVL_ERROR,"[FileCancelAck]file close failed\n");
                wGuiAck = -2;
                file = NULL;
                goto post2gui;
        }


        CheckFileIn((LPCSTR)file_name_path,&tFile);
        tFile->FileStatus = STATUS_CANCELLED;
        tFile->UploadFileSize = m_dwUploadFileSize;
        tFile->FileSize = m_dwFileSize;
        emFileStatus = STATUS_CANCELLED;
	   

post2gui:
        tGuiAck.wGuiAck = wGuiAck;
        strcpy((LPSTR)tGuiAck.FileName,(LPCSTR)file_name_path);
        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_FILE_CANCEL_ACK
               ,&tGuiAck,sizeof(tGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[FileCancelAck]post error\n");
        }
}

void CCInstance::FileGoOnCmd(CMessage* const pMsg){

    TFileList *tFile;
    TDemoInfo tDemoInfo;
	CCInstance *ccIns;

    wGuiAck = 0;
    if(!g_bSignFlag){
            OspLog(SYS_LOG_LEVEL,"[FileGoOnCmd]did not sign in\n");
            return;
    }

    if(!pMsg->content || pMsg->length <= 0){
             OspLog(LOG_LVL_ERROR,"[FileGoOnCmd]Msg is NULL\n");
             return;
    }

    if(!CheckFileIn((LPCSTR)pMsg->content,&tFile)){
            OspLog(LOG_LVL_ERROR,"[FileGoOnCmd]file not in list\n");
            wGuiAck = -12;
            goto postError2gui;
    }

#if 0
    if(tFile->FileStatus >= STATUS_UPLOAD_CMD
                    && tFile->FileStatus <= STATUS_RECEIVE_REMOVE){
                if(OSP_OK != post(MAKEIID(GetAppID(),GetInsID()),FILE_GO_ON_CMD
                               ,pMsg->content,pMsg->length)){
                        OspLog(LOG_LVL_ERROR,"[FileGoOnCmd] post error\n");
                }
                return;

    }
#endif

	if(tFile->FileStatus != STATUS_CANCELLED){
                OspLog(LOG_LVL_ERROR,"[FileGoOnCmd]file upload not cancelled\n");
                wGuiAck = -13;
                goto postError2gui;
	}

    if(!(ccIns = GetPendingIns())){
            OspLog(SYS_LOG_LEVEL, "[FileGoOnCmd]no pending instance,please wait...\n");
            wGuiAck = -14;
            goto postError2gui;
    }
    strcpy((LPSTR)tDemoInfo.FileName,(LPCSTR)tFile->FileName);
    tDemoInfo.UploadFileSize = tFile->UploadFileSize;
    tDemoInfo.FileSize = tFile->FileSize;
    if(OSP_OK != post(MAKEIID(CLIENT_APP_ID,ccIns->GetInsID()),FILE_GO_ON_CMD_DEAL
                   ,&tDemoInfo,sizeof(tDemoInfo))){
            OspLog(LOG_LVL_ERROR,"[FileGoOnCmd] post error\n");
            wGuiAck = -15;
            goto postError2gui;
    }
    ccIns->m_curState = RUNNING_STATE;
    tFile->FileStatus = STATUS_UPLOADING;
    tFile->DealInstance = ccIns->GetInsID();

postError2gui:

        tGuiAck.wGuiAck = wGuiAck;
        strcpy((LPSTR)tGuiAck.FileName,(LPCSTR)pMsg->content);
        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_FILE_GO_ON_ACK
               ,&tGuiAck,sizeof(tGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[FileGoOnCmd]post error\n");
        }
}

void CCInstance::FileGoOnCmdDeal(CMessage* const pMsg){

        TDemoInfo *tDemoInfo;

        tDemoInfo = (TDemoInfo*)pMsg->content;
        strcpy((LPSTR)file_name_path,(LPCSTR)tDemoInfo->FileName);

        if(!g_bSignFlag){
                OspLog(SYS_LOG_LEVEL,"[FileGoOnCmdDeal]did not sign in\n");
                return;
        }

        wGuiAck = 0;
        m_dwUploadFileSize = tDemoInfo->UploadFileSize;
        m_dwFileSize = tDemoInfo->FileSize;

        if(!(file = fopen((LPCSTR)tDemoInfo->FileName,"rb"))){
                OspLog(LOG_LVL_ERROR,"[FileGoOnCmdDeal]open file failed\n");
                wGuiAck = -11;
                goto postError2gui;
        }

        if(fseek(file,m_dwUploadFileSize,SEEK_SET) != 0){
                 OspLog(LOG_LVL_ERROR,"[FileGoOnCmdDeal] file fseeek error\n");
                 wGuiAck = -12;
                 goto postError2gui;
        }

        if(OSP_OK != post(MAKEIID(SERVER_APP_ID,DAEMON),FILE_GO_ON
                       ,(LPCSTR)tDemoInfo->FileName,strlen((LPCSTR)tDemoInfo->FileName)+1,g_dwdstNode)){
                OspLog(LOG_LVL_ERROR,"[FileGoOnCmdDeal] post error\n");
                return;
        }


postError2gui:
        tGuiAck.wGuiAck = wGuiAck;
        strcpy((LPSTR)tGuiAck.FileName,(LPCSTR)file_name_path);
        if(OSP_OK != post(MAKEIID(GUI_APP_ID,DAEMON),GUI_FILE_GO_ON_ACK
               ,&tGuiAck,sizeof(tGuiAck),g_dwGuiNode)){
                OspLog(LOG_LVL_ERROR,"[FileGoOnCmdDeal]post error\n");
        }

}



#if _LINUX_
static bool CheckFileIn(LPCSTR filename,TFileList **tFile){

        struct list_head *tFileHead,*templist;
        TFileList *tnFile = NULL;
        bool inFileList = false;

        list_for_each_safe(tFileHead,templist,&tFileList){
                tnFile = list_entry(tFileHead,TFileList,tListHead);
                if(0 == strcmp((LPCSTR)tnFile->FileName,filename)){
                        inFileList = true;
                        break;
                }
        }
        if(tFile){
                if(inFileList){
                    *tFile = tnFile;
                }else{
                    *tFile = NULL;
                }
        }
        return inFileList;
}
#else
bool CheckFileIn(LPCSTR filename,TFileList **tFile){

        bool inFileList = false;
        list<TFileList*>::iterator iter = tFileList.begin();

        while(iter != tFileList.end()){
                if(0 == strcmp((LPCSTR)((*iter)->FileName),(LPCSTR)filename)){
                        inFileList = true;
                        break;
                }
                iter++;
        }
        if(tFile){
                if(inFileList){
                        *tFile = *iter;
                }else{
                        *tFile = NULL;
                }
        }
        return inFileList;
}
#endif


static CCInstance* GetPendingIns(){

       u16 instCount;
       CCInstance* pIns;

       g_cCApp.wLastIdleInstID %= MAX_INS_NUM;
	   instCount = g_cCApp.wLastIdleInstID;
	   do{
               instCount++;
               pIns = (CCInstance*)((CApp*)&g_cCApp)->GetInstance(instCount);
               if( pIns->CurState() == CInstance::PENDING ) {
                    break;
               }
               instCount %= MAX_INS_NUM;
	   } while( instCount != g_cCApp.wLastIdleInstID );

	   if( instCount == g_cCApp.wLastIdleInstID ){
                return NULL;
       }
	   g_cCApp.wLastIdleInstID = instCount;
       return pIns;

}

