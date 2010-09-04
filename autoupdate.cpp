#include "exception.h"
#include "downloadmanager.h"
#include "xmlliteparser.h"
#include "autoupdate.h"

using namespace std;
#define COMMAND_NAME "AUTOUPDATE"  // AutoUpdate Handler

cAutoUpdate::cAutoUpdate()
{
}

char *cAutoUpdate::GetCommandName()
{
	return COMMAND_NAME;
}

void cAutoUpdate::OnStartup(cIMsgQueue *pMsgQueue, cISysInfo *pSysInfo, cILogger *pLogger)
{
	m_pMsgQueue = pMsgQueue;
	m_pSysInfo = pSysInfo;
}

void cAutoUpdate::OnShutdown()
{
}

int cAutoUpdate::OnProcess(cIMsg *pMessage)
{
	cIMsg *pReply =	m_pMsgQueue->CreateMessageReply(pMessage);		
	pReply->SetContentType("text/xml");
	string sXML = "<Downloader> <GetList url=\"http://localhost/tst/autoupdate.xml\" /></Downloader>";
	pReply->SetContentPayload(sXML.c_str(), sXML.length());

	// Finalize Response
	pReply->SetKeepAlive(false);
	pReply->DispatchMsg();

	// Prepare message for DownloadManager
	cIMsg *pMsg = m_pMsgQueue->CreateInternalMessage();
	pMsg->SetCommand("DWNLDM");

	// Load payload for DownloadManager message
	pMsg->SetContentPayload(sXML.c_str(), sXML.length());

	// Dispatch message for DownloadManager
	pMsg->DispatchMsg();

	pMessage->MarkProcessed();
	return 0;
}

