#ifdef _PLATFORM_WIN32
	#include <windows.h>
#endif //_PLATFORM_WIN32

#ifndef _PLATFORM_WIN32
	#include <unistd.h>
#endif //!_PLATFORM_WIN32

#define LOGFILE "c:\\tszscraper.log"

#include "tszscraper.h"

#include "logger.h"
#include "serveripc.h"
#include "msg.h"
#include "cbstrtok.h"
#include "serveripc.h"
#include "parsemarkup.h"
#include "cbstrtok.h"
#include "xmlliteparser.h"

#include <boost/regex.hpp>

using namespace std;
using namespace boost;

cTSZScraper::cTSZScraper()
{
	m_pCommandName = new char[80];
	strcpy(m_pCommandName, "TSZSCRAPER");
	m_SecondsCount = 120;

	ResetDocs();
}

void cTSZScraper::ResetDocs()
{
	m_xmlDocument.ReplaceWith("<TeamStreamZ>");
	m_xmlDocument.Append("<TeamStreamZ>");
	m_cachedXmlDocument.ReplaceWith(m_xmlDocument.cstr());

	m_cachedCVSDocument[0].Reset();
	m_cachedCVSDocument[1].Reset();
}

char *cTSZScraper::GetCommandName()
{
	return m_pCommandName;
}

void cTSZScraper::OnStartup(cIMsgQueue *pMsgQueue, cISysInfo *pSysInfo, cILogger *pLogger)
{
	m_pMsgQueue = pMsgQueue;
	m_pSysInfo = pSysInfo;
	m_pLogger = pLogger;

	const char *pServerID = pSysInfo->Query("server.uuid");
	if (pServerID)
		m_ServerID = pServerID;


    m_sServerName = m_pSysInfo->Query("server.name");
    m_sServerAddress = m_pSysInfo->Query("server.ip");
    m_sServerPort = "3400";
	m_sServerDirectory = m_pSysInfo->Query("server.directory");
    m_sFrom = m_sServerName;
    m_sFrom += "@";
    m_sFrom += m_sServerAddress;
    m_sFrom += ":";
    m_sFrom += m_sServerPort;

	// Setup for Scraper Init (Recent "old" Stats)
	m_sInitScraperDoc = m_pSysInfo->Query("server.directory");
	m_sInitScraperDoc += "\\init\\jazz\\stats\\stats.html";

	// Setup URLs and Files for Local & Remote Scraping
	m_sLocalScraperFile  = m_pSysInfo->Query("server.directory");
	m_sLocalScraperFile += "\\www\\jazz\\stats\\stats.html";

	m_sLocalScraperUrl     = "http://localhost/jazz/stats/stats.html";					// local
	m_sRemoteScraperUrl[0] = "http://www.nba.com/jazz/stats/";							// nba
	m_sRemoteScraperUrl[1] = "http://teamstreamz.selvasoft.com/jazz/stats/stats.html";	// SELVASOFT
	m_sRemoteScraperUrl[2] = "http://cjus.com/jazz/stats/stats.html";					// CJUS
	bScraperIsLocal=true;			// local or remote
	nIdxRemoteScraperUrl=0;			// idx [0..2] for remote site, nba, cjus, selvasoft

	// For Writing WebDocs - names of docs to write to web
	m_sWebScraperWebDir  = m_pSysInfo->Query("server.directory");
	m_sWebScraperWebDir += "\\www\\jazz\\stats\\";

	m_sWebScraperXml  = m_sWebScraperWebDir;
	m_sWebScraperCsv1 = m_sWebScraperWebDir;
	m_sWebScraperCsv2 = m_sWebScraperWebDir;

	m_sWebScraperXml += "jazz.xml";
	m_sWebScraperCsv1+= "jazz1.csv";
	m_sWebScraperCsv2+= "jazz2.csv";

	// Init Startup Stats
	//ReadFromFile(m_sInitScraperDoc.c_str(),&m_document);
	//ProcessDocument(m_sLocalScraperUrl);

#ifndef _DEBUG
	m_bIsActive = true;								// for demo want to start on command
	bScraperIsLocal=false;							// local or remote
	nIdxRemoteScraperUrl=0;							// idx [0..2] for remote site, nba, selvasoft, cjus
#else
	m_bIsActive = false;							// for demo want to start on command
	bScraperIsLocal=true;							// local or remote
	nIdxRemoteScraperUrl=0;							// idx [0..2] for remote site, nba, selvasoft, cjus
#endif //_DEBUG

	//string sURL = "http://www.nba.com/jazz/stats/";		// NOTE: m_document has document pre-loaded
	//m_bIsActive = (strcmp(m_pSysInfo->Query("server.module.name"), "tszcrawler")==0) ? true : false;

////////////////////////////////////////////////////////////////////////////////////
// Test new scrapers
//
	cBuffer testBuffer;
	int iReturnCode;
	std::string sFil;

	///////////////////////////////////////////////////////////////
	// Get Team Stands
	//
	iReturnCode = GetStandings("www.nba.com", 
								"http://www.nba.com/standings/team_record_comparison/conferenceNew_Std_Div.html", 
								testBuffer);
	sFil  = m_sWebScraperWebDir + "standings.xml";
	SaveToFile(sFil.c_str(), m_cachedXmlDocument);

	sFil  = m_sWebScraperWebDir + "standings.csv";
	SaveToFile(sFil.c_str(), testBuffer);
	
	///////////////////////////////////////////////////////////////
	// Get Carlos Boozer Player Stats
	//
	iReturnCode = GetPlayerStats("www.nba.com", 
								"http://www.nba.com/playerfile/carlos_boozer/index.html", 
								TABLE_ONE, testBuffer);
	sFil  = m_sWebScraperWebDir + "carlos_boozer-t1.csv";
	SaveToFile(sFil.c_str(), testBuffer);

	iReturnCode = GetPlayerStats("www.nba.com", 
								"http://www.nba.com/playerfile/carlos_boozer/index.html", 
								TABLE_TWO, testBuffer);
	sFil  = m_sWebScraperWebDir + "carlos_boozer-t2.csv";
	SaveToFile(sFil.c_str(), testBuffer);

	iReturnCode = GetPlayerStats("www.nba.com", 
								"http://www.nba.com/playerfile/carlos_boozer/index.html", 
								TABLE_THREE, testBuffer);
	sFil  = m_sWebScraperWebDir + "carlos_boozer-t3.csv";
	SaveToFile(sFil.c_str(), testBuffer);
	

	///////////////////////////////////////////////////////////////
	// Get Andrei Kirilenko Player Stats
	//
	iReturnCode = GetPlayerStats("www.nba.com", 
								"http://www.nba.com/playerfile/andrei_kirilenko/index.html", 
								TABLE_ONE, testBuffer);
	sFil  = m_sWebScraperWebDir + "andrei_kirilenko-t1.csv";
	SaveToFile(sFil.c_str(), testBuffer);

	iReturnCode = GetPlayerStats("www.nba.com", 
								"http://www.nba.com/playerfile/andrei_kirilenko/index.html", 
								TABLE_TWO, testBuffer);
	sFil  = m_sWebScraperWebDir + "andrei_kirilenko-t2.csv";
	SaveToFile(sFil.c_str(), testBuffer);

	iReturnCode = GetPlayerStats("www.nba.com", 
								"http://www.nba.com/playerfile/andrei_kirilenko/index.html", 
								TABLE_THREE, testBuffer);
	sFil  = m_sWebScraperWebDir + "andrei_kirilenko-t3.csv";
	SaveToFile(sFil.c_str(), testBuffer);

	
	///////////////////////////////////////////////////////////////
	// Get Jazz News
	//
	iReturnCode = GetJazzRSS("www.nba.com", 
								"http://www.nba.com/jazz/rss.xml", 
								testBuffer);
	sFil  = m_sWebScraperWebDir + "jazz_news.csv";
	SaveToFile(sFil.c_str(), testBuffer);

//q//////////////////////////////////////////////////////////////////////////////////
}

int cTSZScraper::OnProcess(cIMsg *pMessage)
{
 	HandleMessage(pMessage);
    pMessage->MarkProcessed();
	return HRC_COMMAND_HANDLER_OK;
}

const char *cTSZScraper::OnProcessInternal(const char *pMessage)
{
	return "";
}

void cTSZScraper::OnChron()
{
	if (!m_bIsActive) return;
	else OnChron1();
}

void cTSZScraper::DoChron()
{
	// save active, force chron, and restore
	bool buActive = m_bIsActive;
	m_bIsActive = false;	// temporarily exclude
	m_SecondsCount=59;		// force refresh next chron
	OnChron1();				// and manually call onChron1()
	m_bIsActive = buActive;	// Restore state
}

void cTSZScraper::OnChron1()
{
//http://www.nba.com/jazz/stats/
//http://www.nba.com/jazz/roster/
//http://www.nba.com/standings/team_record_comparison/conferenceNew_Std_Div.html
//http://www.nba.com/jazz/news/Jazz_Game_Recaps.html

	m_SecondsCount++;
	if (m_SecondsCount < 60)
	{
		return;
	}
	m_SecondsCount = 0;

	// Determine if Local or Remote Access
	if (bScraperIsLocal)
	{
		ReadFromFile(m_sLocalScraperFile.c_str(),&m_document);
		ProcessDocument(m_sLocalScraperUrl);
	}
	else
	{
		string sURL = "http://www.nba.com/jazz/stats/";
		string sID = "";
		cBuffer domain;
		int rc;
		try
		{
			rc = GetWebDocument("www.nba.com",
					/*sURL.c_str(),*/
					m_sRemoteScraperUrl[nIdxRemoteScraperUrl].c_str(),
					m_document );
			if (rc == 301 || rc == 302)
			{
				//L2C("Doc returned with 301 or 302 code");

				// retry a few times
				int maxtries = 5;
				int iter = 0;
				cBuffer newLocation;
				while (iter++ < maxtries)
				{
					if (m_document.CopyBetween("Location: ","\r\n",newLocation)==0)
					{
						if (!newLocation.IsEmpty())
						{
							sURL = newLocation.c_str();
							GetDomain(newLocation.c_str(), domain);
							if (domain.StringLen() == 0)
								return;

							//L2C("... trying (%d) new location: [%s][%s]", iter, domain.c_str(), sURL.c_str());

							rc = GetWebDocument(domain.cstr(), sURL.c_str(), m_document);
						}
						else
						{
							rc = 0;
							break;
						}
					}
					if (rc != 301 && rc != 302)
					{
						if (rc == 200)
						{
							ProcessDocument(sURL);
						}
						break;
					}					
				}
			}
			else
			{
				if (rc == 200)
				{
					//SaveToFile("c:\\tszscraper.txt",  m_document);
					ProcessDocument(sURL);
				}
			}
		}
		catch (exception const &e)
		{
			L2C(e.what());
		}
		catch (...)
		{
			L2C("Exception in tszscraper::OnChron()");
		}
	}
	L2C("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
}

void cTSZScraper::HandleMessage(cIMsg *pMessage)
{
	// Interpret command line args
	bool bForceXMLOutput = false;
	cIMsg *pReply =	m_pMsgQueue->CreateMessageReply(pMessage);		
	if (pMessage->GetProtocolType() == cIMsg::MSG_PROTOCOL_HTTP_GET)
	{
		// Case sensitive compare 
		if (strstr(pMessage->GetStatusLine(), "csv=2") != 0)
		{
			// ?csv=2 - 2nd stats in csv format
			pReply->SetContentType("text/plain");
			pReply->SetContentPayload(m_cachedCVSDocument[1].c_str(), m_cachedCVSDocument[1].GetBufferCount());
		}
		else if (strstr(pMessage->GetStatusLine(), "action=shutdown") != 0)
		{
			TerminateProcess(GetCurrentProcess(), 0);
			//cCore::GetInstance()->Shutdown();
		}
		else if (strstr(pMessage->GetStatusLine(), "action=show") != 0)
		{
			HWND h=FindWindow(NULL,"msgCourier Server");
			ShowWindow(h, SW_NORMAL);
			UpdateWindow(h);

			// Default to quick text reply
			pReply->SetContentType("text/plain");
			const char *p = "Show command sent";
			pReply->SetContentPayload(p, strlen(p));
		}
		else if (strstr(pMessage->GetStatusLine(), "action=hide") != 0)
		{
			HWND h=FindWindow(NULL,"msgCourier Server");
			ShowWindow(h, SW_HIDE);

			// Default to quick text reply
			pReply->SetContentType("text/plain");
			const char *p = "Hide command sent";
			pReply->SetContentPayload(p, strlen(p));
		}
		else if (strstr(pMessage->GetStatusLine(), "scraper=reset") != 0)
		{
			// ?scraper=reset - Init
			ReadFromFile(m_sInitScraperDoc.c_str(),&m_document);
			string sURL = "http://www.nba.com/jazz/stats/";		// NOTE: m_document has document loaded
			ResetDocs();				// Initialize/reset buffers
			ProcessDocument(sURL);

			// Default to CSV
			pReply->SetContentType("text/plain");
			pReply->SetContentPayload(m_cachedCVSDocument[0].c_str(), m_cachedCVSDocument[0].GetBufferCount());
		}
		else if (strstr(pMessage->GetStatusLine(), "scraper=start") != 0)
		{
			// ?scraper=start - start using chron
			DoChron();
			m_bIsActive = true;		// Activate for next refresh

			// Default to CSV
			pReply->SetContentType("text/plain");
			pReply->SetContentPayload(m_cachedCVSDocument[0].c_str(), m_cachedCVSDocument[0].GetBufferCount());
		}
		else if (strstr(pMessage->GetStatusLine(), "scraper=stop") != 0)
		{
			// ?scraper=stop - stop using chron
			m_bIsActive = false;

			// Default to CSV
			pReply->SetContentType("text/plain");
			pReply->SetContentPayload(m_cachedCVSDocument[0].c_str(), m_cachedCVSDocument[0].GetBufferCount());
		}
		else if (strstr(pMessage->GetStatusLine(), "scraper=local") != 0)
		{
			// ?scraper=local - use local 
			bScraperIsLocal=true;			// local or remote
			DoChron();

			// Default to CSV
			pReply->SetContentType("text/plain");
			pReply->SetContentPayload(m_cachedCVSDocument[0].c_str(), m_cachedCVSDocument[0].GetBufferCount());
		}
		else if (strstr(pMessage->GetStatusLine(), "scraper=remote0") != 0)
		{
			// ?scraper=remote0 - use remote 
			bScraperIsLocal=false;			// local or remote
			nIdxRemoteScraperUrl=0;			// idx [0..2] for remote site, nba, selvasoft, cjus
			DoChron();

			// Default to CSV
			pReply->SetContentType("text/plain");
			pReply->SetContentPayload(m_cachedCVSDocument[0].c_str(), m_cachedCVSDocument[0].GetBufferCount());
		}
		else if (strstr(pMessage->GetStatusLine(), "scraper=remote1") != 0)
		{
			// ?scraper=remote1 - use remote 
			bScraperIsLocal=false;			// local or remote
			nIdxRemoteScraperUrl=1;			// idx [0..2] for remote site, nba, selvasoft, cjus
			DoChron();

			// Default to CSV
			pReply->SetContentType("text/plain");
			pReply->SetContentPayload(m_cachedCVSDocument[0].c_str(), m_cachedCVSDocument[0].GetBufferCount());
		}
		else if (strstr(pMessage->GetStatusLine(), "scraper=remote2") != 0)
		{
			// ?scraper=remote2 - use remote 
			bScraperIsLocal=false;			// local or remote
			nIdxRemoteScraperUrl=2;			// idx [0..2] for remote site, nba, selvasoft, cjus
			DoChron();

			// Default to CSV
			pReply->SetContentType("text/plain");
			pReply->SetContentPayload(m_cachedCVSDocument[0].c_str(), m_cachedCVSDocument[0].GetBufferCount());
		}
		else if (strstr(pMessage->GetStatusLine(), "xml") != 0)
		{
			// ?xml - send XML
			pReply->SetContentType("text/xml");
			pReply->SetContentPayload(m_cachedXmlDocument.c_str(), m_cachedXmlDocument.GetBufferCount());
		}
		else // if (strstr(pMessage->GetStatusLine(), "csv=1") != 0)
		{
			// ?csv=1 + default - csv
			pReply->SetContentType("text/plain");
			pReply->SetContentPayload(m_cachedCVSDocument[0].c_str(), m_cachedCVSDocument[0].GetBufferCount());
		}
	}
	
	// Isolate POST if needed
	// if (pMessage->GetProtocolType() == cIMsg::MSG_PROTOCOL_HTTP_POST) ...

	// Finalize Response
	pReply->SetKeepAlive(false);
	pReply->DispatchMsg();
	pMessage->MarkProcessed();
}

int cTSZScraper::ProcessDocument(string &sURL)
{
	bool bUpdated = false;

	m_xmlDocument.ReplaceWith("<TeamStreamZ>");

	int idx = 0;
	int idx2 = 0;
	int cvsDoc = 0;

	cBuffer firstSectionTitle;
	cBuffer secondSectionTitle;
	//if (sURL == "http://www.nba.com/jazz/stats/")
	{

		while (idx != -1)
		{
			if (cvsDoc == 0)
			{
				m_cvsDocument[cvsDoc].Reset();
				m_cvsDocument[cvsDoc].Append("Player,G,GS,MPG,FG%25,3p%25,FT%25,OFF,DEF,TOT,APG,SPG,BPG,TO,PF,PPG\n");
			}
			else if (cvsDoc == 1)
			{
				m_cvsDocument[cvsDoc].Reset();
				m_cvsDocument[cvsDoc].Append("Player,G,GS,MIN,FGM-A,3PM-A,FTM-A,OFF,DEF,TOT,AST,STL,BLK,TO,PF,PTS\n");
			}

			idx = m_document.Find("<div class=\"gSGSectionTitle\">", idx);
			if (idx != -1)
			{
				idx += 29; // length of last search pattern
				idx2 = m_document.FindChar('\r', idx);
				firstSectionTitle.ReplaceWith((char*)m_document.c_str() + idx, idx2 - idx);
				firstSectionTitle.ReplaceAll("&nbsp;","");
				m_xmlDocument.Append("<Table src=\"");
				m_xmlDocument.Append((char*)sURL.c_str());
				m_xmlDocument.Append("\" name=\"");
				m_xmlDocument.Append((char*)firstSectionTitle.c_str());
				m_xmlDocument.Append("\" >");

				int nextSectionIndex = m_document.Find("created:", idx);

				m_xmlDocument.Append("<PlayerCollection>");
				while ((idx = m_document.Find("<a class=gSGPlayerLink", idx2)) < nextSectionIndex)
				{
					if (idx != -1)
					{
						idx2 = m_document.Find("</tr>", idx+1);
						if (idx2 != -1)
						{
							// process player info
							//
							cBuffer markup;
							markup.ReplaceWith((char*)m_document.c_str()+idx, idx2 - idx);
							cParseMarkup pm;
							pm.Parse(markup.c_str(), true);
							const char *pData = pm.GetText();
							STRTOKVA;
							char *pSep = " \r\n\t";
							char *pToken;
							string firstName;
							string lastName;
							pToken = STRTOK((char*)pData, pSep);
							if (pToken != NULL)
								firstName = pToken;
							pToken = STRTOK(NULL, pSep);
							if (pToken != NULL)
								lastName = pToken;						
							if (pToken != NULL)
							{
								m_xmlDocument.Append("<Player name=\"");
								m_xmlDocument.Append((char*)firstName.c_str());
								m_xmlDocument.Append(" ");
								m_xmlDocument.Append((char*)lastName.c_str());
								m_xmlDocument.Append("\" ");

								m_cvsDocument[cvsDoc].Append((char*)firstName.c_str());
								m_cvsDocument[cvsDoc].Append(" ");
								m_cvsDocument[cvsDoc].Append((char*)lastName.c_str());

								int cnt = 1;
								char szBuf[256];
								while (pToken != NULL)
								{
									pToken = STRTOK(NULL, pSep);
									if (pToken != NULL)
									{
										sprintf(szBuf,"d%d=\"%s\" ", cnt, pToken);
										m_xmlDocument.Append(szBuf);

										m_cvsDocument[cvsDoc].Append(",");
										//*cg* should apply escape here
										m_cvsDocument[cvsDoc].Append(pToken);

										cnt++;
									}
								}
								m_xmlDocument.Append("/>");
								m_cvsDocument[cvsDoc].Append("\n");
							}						
						}
					}
					else
					{
						break;
					}
				}

				idx = idx2;
				m_xmlDocument.Append("</PlayerCollection>");
				m_xmlDocument.Append("</Table>");

				m_cvsDocument[cvsDoc].Append("\n");
				bUpdated = true;
			}

			cvsDoc++;
		}
	}

	m_xmlDocument.Append("</TeamStreamZ>");

	if (bUpdated == true)
	{
		// update cached document
		//
		m_cachedXmlDocument.ReplaceWith(m_xmlDocument.cstr());
		m_cachedCVSDocument[0].ReplaceWith(m_cvsDocument[0].cstr());
		m_cachedCVSDocument[1].ReplaceWith(m_cvsDocument[1].cstr());

		// Save Docs to WWW
		SaveToFile(m_sWebScraperXml.c_str(), m_cachedXmlDocument);
		SaveToFile(m_sWebScraperCsv1.c_str(), m_cvsDocument[0]);
		SaveToFile(m_sWebScraperCsv2.c_str(), m_cvsDocument[1]);

	}
	return 0;
}

void cTSZScraper::OnShutdown()
{
	delete []m_pCommandName;
}

int cTSZScraper::GetWebDocument(const char *pDomain, const char *pURL, cBuffer &document)
{
	m_document.Reset();
	m_cvsDocument[0].Reset();
	m_cvsDocument[1].Reset();

	cServerIPC host;
	cBuffer message;
	cBuffer path;
	char *pPath;	
	int httpReturnCode = 0;
 
	GetResourcePath(pURL, path);
	if (path.StringLen()>1)
		pPath = path.cstr();
	else
		pPath = "/";
	
	int rc = host.Open((char*)pDomain, 80);
	if (rc == HRC_SERVERIPC_SUCCESS)
	{
		message.Sprintf(8096, "GET %s HTTP/1.1\r\nConnection: close\r\nUser-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.0.6) Gecko/20060728 Firefox/1.5.0.6\r\nHost: %s\r\n\r\n", pPath, pDomain);

		host.Send(message.cstr(), message.StringLen());
		//L2C("send header:\n%s", message.c_str());
		host.CheckForResponse(15);
		m_document.ReplaceWith(host.GetResponse());
		//L2C("%s\n\n", m_document.c_str());
		host.Close();
	}
	host.Reset();

   	int iResponseFirstLine = m_document.Find("\r\n",0);
	if (iResponseFirstLine != -1)
   	{
   		cBuffer firstLine;
   		firstLine.ReplaceWith(m_document.cstr(), iResponseFirstLine);
	   	//L2C("Status: %s\n", firstLine.c_str());
		if (strstr(firstLine.c_str(), "200 ")!=0) 
			httpReturnCode = 200;
		else if (strstr(firstLine.c_str(), "301 ")!=0) 
			httpReturnCode = 301;
		else if (strstr(firstLine.c_str(), "302 ")!=0) 
			httpReturnCode = 302;
		else if (strstr(firstLine.c_str(), "404 ")!=0) 
			httpReturnCode = 404;
	}

	document.ReplaceWith(m_document.cstr(), m_document.GetBufferCount());
	return httpReturnCode;	
}

void cTSZScraper::GetDomain(const char *pURL, cBuffer &domain)
{
   	domain.ReplaceWith((char*)pURL);
	domain.Replace("http://", "");
	int iPos = domain.FindChar('/',0);
	if (iPos != -1)
	{
		char *p = domain.cstr();
		p[iPos] = 0;			 
		domain.MakeString();				
	}
	else
	{
		//domain.Reset();
	}
}

void cTSZScraper::GetResourcePath(const char *pURL, cBuffer &path)
{
   	path.ReplaceWith((char*)pURL);
    path.Replace("http://www.", "");
	path.Replace("http://", "");
	path.Replace("www.", "");
	int iPos = path.FindChar('/',0);
	if (iPos == 0)
	{
		// just use it
	}
	else if (iPos > 1)
	{
		path.Remove(0, iPos);
		path.MakeString();				
	}
	else
	{
		path.ReplaceWith("/");		
	}   
}

void cTSZScraper::ReadFromFile(const char* pFileName, cBuffer* buffer)
{
	buffer->Reset();
	FILE *fp = fopen(pFileName,"rb");	// assume <cr><lf> willbe returned as <cr><lf>
	if (!fp)
	{
		fclose(fp);	
		return;
	}
	char rdbuf[1024];
	size_t chrs;
	while ((chrs=fread((void*)rdbuf,1,1024,fp))>0)
	{
		buffer->Append(rdbuf,chrs);
	}
	fclose(fp);	
}


void cTSZScraper::SaveToFile(const char* pFileName, cBuffer &buffer)
{
	FILE *fp = fopen(pFileName,"wt");
	if (!fp)
	{
		fclose(fp);	
		return;
	}
	fwrite(buffer.cstr(), buffer.StringLen(), 1, fp);
	fclose(fp);	
}

void cTSZScraper::Location(const char *pFile, int linenum)
{
	L2C("[%s::%d]", pFile, linenum);
}

void cTSZScraper::L2C(const char *pFormat, ...)
{
	va_list ap;
	va_start(ap, pFormat);
	int iCnt = vsprintf(m_CharBuffer, pFormat, ap);
	va_end(ap);

#ifdef LOGFILE
	time_t t = time(0);
	tm *t2 = localtime(&t);
	FILE *fp = 0;
	fp = fopen(LOGFILE,"a");
	if (fp)
		fprintf(fp, "%.2d%.2d-%.2d%.2d%.2d: %s\n", t2->tm_mon+1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec , m_CharBuffer);
	fclose(fp);
#endif //LOGFILE

	if (iCnt > 256)
	{
		m_CharBuffer[iCnt] = 0;
		strcat(m_CharBuffer, "...");
	}
	m_pLogger->Write(m_CharBuffer);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int cTSZScraper::GetStandings(const char *pDomain, const char *pURL, cBuffer &Buffer)
{
	// http://www.nba.com/standings/team_record_comparison/conferenceNew_Std_Div.html
	cBuffer csv;
	Buffer.Reset();
	int rc = GetWebDocument(pDomain, pURL, Buffer); 
	if (rc == 200)
	{
		cBuffer content;
		cBuffer table;
		cBuffer row;
		cBuffer field;
		cBuffer title;
		cBuffer sup;
		int rowOffset = 0;
		int fieldOffset = 0;

		int offset = Buffer.Find("created: ");
		if (offset > 0)
		{
			ExtractContent(Buffer, offset, "</td>", "</table>", table);
			while ((rowOffset = ExtractContent(table, rowOffset, "<tr", "</tr>", row)) > 0)
			{
				while ((fieldOffset = ExtractContent(row, fieldOffset, "<td", "</td>", field)) > 0)
				{
					if (strstr(field.c_str(), "Conference") != 0)
					{
						field.Cleanup();
						csv.Append(field.cstr());
						csv.Append(",,,,,,,,,,,");
						break;
					}
					else
					{
						ExtractContent(field, 0, "<a", "</a>", title);
						ExtractContent(field, 0, "<sup", "</sup>", sup);

						if (sup.StringLen() == 0 && title.StringLen() == 0)
						{
							field.Cleanup();
							csv.Append(field.cstr());
						}
						else if (sup.StringLen() > 0)
						{
							title.Cleanup();
							sup.Cleanup();
							csv.Append(title.cstr());
							csv.Append("(");
							csv.Append(sup.cstr());
							csv.Append(")");
						}
						else
						{
							title.Cleanup();
							csv.Append(title.cstr());
						}
						csv.Append(",");
					}
				}
				csv.Truncate(1);
				csv.Append("\n");
				fieldOffset = 0;
			}
		}
	}

	Buffer.ReplaceWith(csv.cstr(), csv.GetBufferCount());
	return rc;
}

int cTSZScraper::GetPlayerStats(const char *pDomain, const char *pURL, int tableNumber, cBuffer &Buffer)
{
	//playerInfoPos
	//<div id="playerInfoB">
	//playerInfoStatsPlayerInfoBorders
	//gSGSectionTitleStatsGrid
	// http://www.nba.com/playerfile/carlos_boozer/index.html
	cBuffer csv;
	cBuffer content;
	cBuffer table;
	cBuffer row;
	cBuffer field;
	cBuffer title;
	cBuffer sup;
	int rowOffset = 0;
	int fieldOffset = 0;

	Buffer.Reset();
	int rc = GetWebDocument(pDomain, pURL, Buffer); 
	if (rc == 200)
	{
		if (tableNumber == TABLE_ONE)
		{
			int offset = Buffer.Find("playerInfoPos");
			if (offset > 0)
			{
				csv.Append("Name,Number,Position,Period,PPG,RPG,APG,EFF,Born,Height,Weight,College,Years Pro\n");
				offset = ExtractContent(Buffer, offset, "<li", "</li>", field);
				field.Cleanup();
				csv.Append(field.cstr());
				csv.Append(",");

				offset = ExtractContent(Buffer, offset, "<li", "</li>", field);
				field.Cleanup();
				csv.Append(field.cstr());
				csv.Append(",");

				offset = ExtractContent(Buffer, offset, "<li", "</li>", field);
				field.Cleanup();
				csv.Append(field.cstr());
				csv.Append(",");

				offset = Buffer.Find("playerInfoB");
				if (offset > 0)
				{
					offset = ExtractContent(Buffer, offset, "<h3", "</", field);
					field.Cleanup();
					csv.Append(field.cstr());
					csv.Append(",");

					for (int i = 0; i < 4; i++)
					{
						offset = ExtractContent(Buffer, offset, "<dd", "</dd>", field);
						field.Cleanup();
						csv.Append(field.cstr());
						csv.Append(",");
					}
				}

				offset = Buffer.Find("playerInfoStatsPlayerInfoBorders");
				if (offset > 0)
				{
					offset = ExtractContent(Buffer, offset, "<span ", "</", field);
					field.Cleanup();
					field.ReplaceAll(",","");
					csv.Append(field.cstr());
					csv.Append(",");

					for (int i=0; i<3; i++)
					{
						offset = ExtractContent(Buffer, offset, "<span ", "</", field);
						field.Cleanup();
						field.ReplaceAll(","," ");
						field.ReplaceAll("&nbsp;"," ");

						csv.Append(field.cstr());
						csv.Append(",");
					}

					offset = ExtractContent(Buffer, offset, "<span ", "</", field);
					field.Cleanup();
					csv.Append(field.cstr());
				}

				csv.Append("\n");
			}		
		}
		else if (tableNumber == TABLE_TWO)
		{
			int offset = Buffer.Find("gSGSectionTitleStatsGrid");
			if (offset > 0)
			{
				ExtractContent(Buffer, offset, "<table", "</table>", table);
				while ((rowOffset = ExtractContent(table, rowOffset, "<tr", "</tr>", row)) > 0)
				{
					while ((fieldOffset = ExtractContent(row, fieldOffset, "<td", "</td>", field)) > 0)
					{
						field.ReplaceAll(","," ");
						field.ReplaceAll("&nbsp;"," ");
						csv.Append(field.cstr());
						csv.Append(",");
					}
					csv.Truncate(1);
					csv.Append("\n");
					fieldOffset = 0;
				}
			}
		}
		else if (tableNumber == TABLE_THREE)
		{
			int offset = Buffer.Find("end career stats");
			offset = Buffer.Find("gSGSectionTitleStatsGrid", offset);
			if (offset > 0)
			{
				ExtractContent(Buffer, offset, "<table", "</table>", table);
				while ((rowOffset = ExtractContent(table, rowOffset, "<tr", "</tr>", row)) > 0)
				{
					while ((fieldOffset = ExtractContent(row, fieldOffset, "<td", "</td>", field)) > 0)
					{
						if (strstr(field.c_str(), "<a ") != 0)
						{
							cBuffer field2;
							ExtractContent(field, 0, "<a ", "</a>", field2);
							field.ReplaceWith(field2.cstr());
						}
						field.ReplaceAll(","," ");
						field.ReplaceAll("&nbsp;"," ");
						csv.Append(field.cstr());
						csv.Append(",");
					}
					csv.Truncate(1);
					csv.Append("\n");
					fieldOffset = 0;
				}
			}
		}
	}

	Buffer.ReplaceWith(csv.cstr(), csv.GetBufferCount());
	return rc;
}

int cTSZScraper::GetJazzRSS(const char *pDomain, const char *pURL, cBuffer &Buffer)
{
	// http://www.nba.com/jazz/rss.xml
	cBuffer csv;
	Buffer.Reset();
	int rc = GetWebDocument(pDomain, pURL, Buffer); 
	if (rc == 200)
	{
		// created:
		int loc = Buffer.Find((char*)"<?xml");
		if (loc > -1)
		{
			cXMLLiteParser xml;
			cXMLLiteElement *pElm; 
			xml.Parse(Buffer.c_str());
			xml.MoveHead();
			cBuffer temp;
			csv.Append("Title,Link,Description\n");
			pElm = xml.FindTag("item");
			do
			{
				if (pElm != 0)
				{
					pElm = xml.FindNextTag("title");
					if (pElm != 0 && pElm->hasCDATA())
					{
						temp.ReplaceWith((char*)pElm->GetElementCDATA());
						csv.Append((char*)EscapeData(temp));
						csv.Append(",");
					}
					pElm = xml.FindNextTag("link");
					if (pElm != 0)
					{
						temp.ReplaceWith((char*)pElm->GetElementValue());
						csv.Append((char*)EscapeData(temp));
						csv.Append(",");
					}
					pElm = xml.FindNextTag("description");
					if (pElm != 0 && pElm->hasCDATA())
					{
						temp.ReplaceWith((char*)pElm->GetElementCDATA());
						csv.Append((char*)EscapeData(temp));
					}
					csv.Append("\n");
				}
			} while ((pElm = xml.FindNextTag("item")) != 0);
		}						
	}
	Buffer.ReplaceWith(csv.cstr(), csv.GetBufferCount());
	return rc;
}

int cTSZScraper::ExtractContent(cBuffer &Buffer, int offset, const char *pStartTag, const char *pEndTag, cBuffer &content)
{
	int ret = 0;
	content.Reset();
	int loc = Buffer.Find((char*)pStartTag, offset);
	if (loc > -1)
	{
		int loc2 = Buffer.Find((char*)pEndTag, loc+1);
		if (loc2 > -1)
		{
			if (strstr(pStartTag, ">") == 0)
				loc = Buffer.Find((char*)">", loc+1);
			content.ReplaceWith(Buffer.cstr() + loc+1, loc2 - loc - 1);
			ret = loc2 + 1;
		}
	}
	return ret;
}

const char *cTSZScraper::EscapeData(cBuffer &buffer)
{
	buffer.ReplaceAll("%", "%25");
	buffer.ReplaceAll("&nbsp;", "%20");
	buffer.ReplaceAll(",", "%2C");
	return buffer.c_str();
}
