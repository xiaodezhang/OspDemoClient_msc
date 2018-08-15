#ifndef _OSP_CLIENT_H
#define _OSP_CLIENT_H

#include"osp.h"
#include"commondemo.h"

#define MULTY_APP                0
#define SINGLE_APP               1

#define RUNNING_STATE                 (1)
#define IDLE_STATE                    (0)
#define OSP_AGENT_CLIENT_PORT         (20001)
#define EV_CLIENT_TEST_BGN           ((u16)0x1111)
#define SERVER_CONNECT_TEST          (EV_CLIENT_TEST_BGN+1)
#define CLIENT_APP_ID                (u16)3
#define MAX_MSG_WAITING              (u32)512
#define CLIENT_APP_PRI               (u8)80
#define SIGN_STATUS_IN                (1)
#define SIGN_STATUS_OUT               (0)
#define AUTHORIZATION_NAME_SIZE       (20)
#define MAX_IP_LENGTH                 (16)
#define SERVER_IP                    "172.16.236.241"
#define SERVER_PORT                  ((u16)20000)


#if 1
#define BUFFER_SIZE                   (u16)(MAX_MSG_LEN >> 1)
#else
#define BUFFER_SIZE                   (u16)1
#endif
#define MAX_FILE_NAME_LENGTH         200

#if MULTY_APP

#define CLIENT_INSTANCE_NUM          1

#endif

#define CLIENT_INSTANCE_ID           1


#define SEND_REMOVE                    (EV_CLIENT_TEST_BGN+17)
#define SEND_CANCEL                    (EV_CLIENT_TEST_BGN+18)
#define FILE_REMOVE_ACK                (EV_CLIENT_TEST_BGN+19)
#define FILE_CANCEL_ACK                (EV_CLIENT_TEST_BGN+20)

#define SEND_REMOVE_CMD                (EV_CLIENT_TEST_BGN+21)
#define SEND_CANCEL_CMD                (EV_CLIENT_TEST_BGN+22)

#define FILE_GO_ON_CMD                 (EV_CLIENT_TEST_BGN+23)
#define FILE_GO_ON                     (EV_CLIENT_TEST_BGN+24)


#define FILE_STABLE_REMOVE             (EV_CLIENT_TEST_BGN+27)
//#define FILE_STABLE_REMOVE_ACK         (EV_CLIENT_TEST_BGN+28)

#define SIGN_IN_CMD                    (EV_CLIENT_TEST_BGN+29)

#if MULTY_APP

#define MY_CONNECT                     (EV_CLIENT_TEST_BGN+30)
#define MY_SIGNED                      (EV_CLIENT_TEST_BGN+32)
#define MY_DISSIGNED                   (EV_CLIENT_TEST_BGN+33)
#define GET_MY_CONNECT                 (EV_CLIENT_TEST_BGN+34)
#define GET_DISCONNECT                 (EV_CLIENT_TEST_BGN+35)
#define GET_SIGNED                     (EV_CLIENT_TEST_BGN+36)
#define GET_DISSIGNED                  (EV_CLIENT_TEST_BGN+37)

#endif

#define FILE_UPLOAD_CMD_DEAL           (EV_CLIENT_TEST_BGN+38)
#define SEND_CANCEL_CMD_DEAL           (EV_CLIENT_TEST_BGN+39)
#define FILE_GO_ON_CMD_DEAL            (EV_CLIENT_TEST_BGN+40)
#define SEND_REMOVE_CMD_DEAL           (EV_CLIENT_TEST_BGN+41)
#define SEND_STABLE_REMOVE_CMD_DEAL    (EV_CLIENT_TEST_BGN+42)


#define MAKEESTATE(state,event) ((u32)((event) << 4 + (state)))

#define STATUS_INIT                     (s32)-1
#define STATUS_RECEIVE_UPLOAD           (s32)9
#define STATUS_RECEIVE_CANCEL           (s32)10
#define STATUS_RECEIVE_REMOVE           (s32)12
#define STATUS_UPLOADING                (s32)13
#define STATUS_CANCELLED                (s32)14
#define STATUS_REMOVED                  (s32)15
#define STATUS_FINISHED                 (s32)16



typedef struct tagSinInfo{
        s8 Username[AUTHORIZATION_NAME_SIZE];
        s8 Passwd[AUTHORIZATION_NAME_SIZE];
}TSinInfo;

typedef struct tagGuiAck{
        s16                    wGuiAck;
        u32                    dwFileSize;
        u32                    dwUploadFileSize;
        u8                     FileName[MAX_FILE_NAME_LENGTH];
}TGuiAck;

typedef enum tagEM_FILE_STATUS{
                STATUS_INIT             = -1,
                //processing state
                STATUS_RECEIVE_UPLOAD   = 9,
                STATUS_RECEIVE_CANCEL   = 10,
                STATUS_RECEIVE_REMOVE   = 12,
                //stable state
                STATUS_UPLOADING        = 13,
                STATUS_CANCELLED        = 14,
                STATUS_REMOVED          = 15,
                STATUS_FINISHED         = 16
}EM_FILE_STATUS;

class CCInstance : public CInstance{

public:
        typedef void (CCInstance::*MsgProcess)(CMessage *const pMsg);
private:

        typedef struct tagCmdNode{
                u32         EventState;
                CCInstance::MsgProcess  c_MsgProcess;
                struct      tagCmdNode *next;
        }tCmdNode;


        u32         m_dwDisInsID;
        u8          file_name_path[MAX_FILE_NAME_LENGTH];

        s8          buffer[BUFFER_SIZE];
        u32         m_dwUploadFileSize;
        u32         m_dwFileSize;      
        u8          m_byServerIp[MAX_IP_LENGTH];
        u16         m_wServerPort;
#if MULTY_APP
        bool        m_bConnectedFlag;    //避免使用线程锁，将全局变量改为成员变量
        bool        m_bSignFlag;         //避免使用线程锁，将全局变量改为成员变量
#endif
private:
        void InstanceEntry(CMessage *const);
        void DaemonInstanceEntry(CMessage *const,CApp*);
        tCmdNode *m_tCmdChain;
        tCmdNode *m_tCmdDaemonChain;
        FILE *file;
public:
        CCInstance(): m_dwDisInsID(0) 
			         ,file(NULL)
					 ,m_dwFileSize(0)
                     ,m_dwUploadFileSize(0)
                     ,m_tCmdChain(NULL)
                     ,m_tCmdDaemonChain(NULL),emFileStatus(STATUS_INIT)
                     ,m_wServerPort(SERVER_PORT){
                memset(file_name_path,0,sizeof(u8)*MAX_FILE_NAME_LENGTH);
                memset(buffer,0,sizeof(u8)*BUFFER_SIZE);
                memcpy(m_byServerIp,SERVER_IP,sizeof(SERVER_IP));
                MsgProcessInit();
        }
        ~CCInstance(){
                NodeChainEnd();
                if(file){
                        fclose(file);
                }
        }
        void MsgProcessInit();
        void NodeChainEnd();
        bool RegMsgProFun(u32,MsgProcess,tCmdNode**);
        bool FindProcess(u32,MsgProcess*,tCmdNode*);


        //注册处理函数
        void SignInCmd(CMessage* const);
        void SignInAck(CMessage* const);
        void SignOutCmd(CMessage* const);
        void SignOutAck(CMessage* const);



        void FileReceiveUploadAck(CMessage* const);

        void FileUploadCmd(CMessage* const);

        void FileUploadAck(CMessage* const);
        void FileFinishAck(CMessage* const);

        void RemoveCmd(CMessage* const);
        void CancelCmd(CMessage* const);
        void FileCancelAck(CMessage* const);
        void FileRemoveAck(CMessage* const);
        void FileGoOnCmd(CMessage* const);
        void FileUploadCmdDeal(CMessage* const);
        void SendCancelCmdDeal(CMessage* const);
        void CancelCmdDeal(CMessage* const);
        void FileGoOnCmdDeal(CMessage* const);
        void RemoveCmdDeal(CMessage* const);
        void StableRemoveCmdDeal(CMessage* const);

#if MULTY_APP
        void GetDisconnect(CMessage* const);
        void notifyConnect(CMessage* const);
        void GetMyConnect(CMessage* const);
        void notifySigned(CMessage* const);
        void notifyDissigned(CMessage* const);
        void GetDissigned(CMessage* const);
        void GetSigned(CMessage* const);
#endif
        //断链检测处理函数
        void notifyDisconnect(CMessage* const);


};

#if _MSC_VER
class TFileList{
public:

        u8                     FileName[MAX_FILE_NAME_LENGTH];
        s32                    FileStatus;
        u16                    DealInstance;
        u32                    UploadFileSize;
        u32                    FileSize;

        inline bool operator == (const TFileList &file) const{
                return (strcmp((LPCSTR)file.FileName,(LPCSTR)FileName) == 0);
        }
};
#endif

#if MULTY_APP
typedef zTemplate<CCInstance,CLIENT_INSTANCE_NUM,CAppNoData,MAX_ALIAS_LENGTH> CCApp;
#else
typedef zTemplate<CCInstance,MAX_INS_NUM,CAppNoData,MAX_ALIAS_LENGTH> CCApp;
#endif

API int clientInit(u32);
#endif //_OSP_CLIENT_H
