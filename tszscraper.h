#ifndef _TSZSCRAPER_H_
#define _TSZSCRAPER_H_

#ifdef _PLATFORM_WIN32
	#ifdef MSGHANDLER_EXPORTS
		#define MSGHANDLER_API __declspec(dllexport)
	#else
		#define MSGHANDLER_API __declspec(dllimport)
	#endif
#endif

#include <string>
#include <map>
#include <list>

#include "msgCourierPlugin.h"
#include "buffer.h"
#include "xmlliteparser.h"

//#include "core.h"
//#define L Location(__FILE__,__LINE__);

#define HRC_TSZSCRAPER_OK						0x0000
#define HRC_TSZSCRAPER_UNABLE_TO_PARSE_DOC		0x0001

class cTSZScraper : public cICommandHandler
{
public:
	cTSZScraper();
	char *GetCommandName();

	enum {TABLE_ONE=1, TABLE_TWO=2, TABLE_THREE=3};

	void ResetDocs();
	void OnStartup(cIMsgQueue *pMsgQueue, cISysInfo *pSysInfo, cILogger *pLogger);
	int OnProcess(cIMsg *pMessage);
	const char *OnProcessInternal(const char *pMessage);
	void OnChron();
	void DoChron();
	void OnChron1();
	void OnShutdown();

private:
	bool m_bIsActive;
	
	cIMsgQueue *m_pMsgQueue;
	cISysInfo *m_pSysInfo;
	cILogger *m_pLogger;

	int m_SecondsCount;
	char *m_pCommandName;
	char m_CharBuffer[65000];
	std::string m_ServerID;
    std::string m_sFrom;
    std::string m_sServerName;
    std::string m_sServerAddress;
    std::string m_sServerPort;
	std::string m_sServerDirectory;

	// Name of Stats files
	std::string m_sInitScraperDoc;				// Recent ("old") stats for demo
	std::string m_sLocalScraperFile;			// www/jazz/stats/stats.html
	std::string m_sLocalScraperUrl;				// www/jazz/stats/stats.html
	std::string m_sRemoteScraperUrl[3];			// nba, selvasoft, cjus
	bool bScraperIsLocal;						// local or remote
	int	 nIdxRemoteScraperUrl;					// idx [0..2] for remote site

	// Names of docs to write to web
	std::string m_sWebScraperWebDir;
	std::string m_sWebScraperXml;
	std::string m_sWebScraperCsv1;
	std::string m_sWebScraperCsv2;
	
    std::string m_FormMessage;
    char m_ImageBuffer[1024];
    int m_ImageBufferSize;
    char m_ipc[1024];
    cBuffer m_Response;

	cBuffer m_document;
	cBuffer m_xmlDocument;
	cBuffer m_cvsDocument[2];
	cBuffer m_cachedXmlDocument;
	cBuffer m_cachedCVSDocument[2];

	void HandleMessage(cIMsg *pMessage);

	int GetWebDocument(const char *pDomain, const char *pURL, cBuffer &document);
	void GetDomain(const char *pURL, cBuffer &domain);
	void GetResourcePath(const char *pURL, cBuffer &path);

	int ProcessDocument(std::string &sURL);
	void StoreDocument(const char *pDomainName, const char *pURL, const char *pTitle, const char *pFullText, const char *pPreProcessedText, int size, double score);

	void ReadFromFile(const char* pFileName, cBuffer* buffer);
	void SaveToFile(const char* pFileName, cBuffer &buffer);
	void Location(const char *pFile, int linenum);
	void L2C(const char *pFormat, ...);


	int GetStandings(const char *pDomain, const char *pURL, cBuffer &Buffer);
	int GetPlayerStats(const char *pDomain, const char *pURL, int tableNumber, cBuffer &Buffer);
	int GetJazzRSS(const char *pDomain, const char *pURL, cBuffer &Buffer);

	int ExtractContent(cBuffer &Buffer, int offset, const char *pStartTag, const char *pEndTag, cBuffer &content);
	const char *EscapeData(cBuffer &buffer);
};

#endif //_TSZSCRAPER_H_
