#ifndef AUTOUPDATE_H
#define AUTOUPDATE_H

#include "msgCourierPlugin.h"

class cAutoUpdate : cICommandHandler
{
public:
	cAutoUpdate();

	char *GetCommandName();

	void OnStartup(cIMsgQueue *pMsgQueue, cISysInfo *pSysInfo, cILogger *pILogger);
	int  OnProcess(cIMsg *pMessage);
	void OnShutdown();

private:
	cIMsgQueue *m_pMsgQueue;
	cISysInfo *m_pSysInfo;

};

#endif //AUTOUPDATE_H

