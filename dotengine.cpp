/* dotengine.cpp
   Copyright (C) 2005 Carlos Justiniano

dotengine.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

dotengine.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with dotengine.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file dotengine.cpp 
 @brief cDotEngine Invokes and serializes access to ATT's GraphViz
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 cDotEngine Invokes and serializes access to ATT's GraphViz
*/
#include <time.h>
#include <sstream>
#include "dotengine.h"
#include "log.h"
#include "exception.h"
#include "uniqueidprovider.h"

using namespace std;

cDotEngine::cDotEngine(std::string &dotpath, std::string &WebDotFolder)
{
	SetThreadName("cDotEngine");
	try
	{
		m_dotpath = dotpath;
		m_WebDotFolder = WebDotFolder;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

cDotEngine::~cDotEngine()
{
}

int cDotEngine::Run()
{
	m_Timer.SetInterval(3600);
	m_Timer.Start();

    while (ThreadRunning())
    {
		m_ThreadSync.Lock();

        if (m_Timer.IsReady())
		{
			m_Timer.Reset();
			// TODO: Cleanup old image files
		}

		m_ThreadSync.Unlock();
		this->Sleep(250);
    }
    return HRC_DOTENGINE_OK;
}

int cDotEngine::Start()
{
	int iRet = HRC_DOTENGINE_OK;
	cThread::Create();
	cThread::Start();
	return iRet;
}

int cDotEngine::Stop()
{
	int iRet = HRC_DOTENGINE_OK;      
	cThread::Destroy();
	return iRet;
}

int cDotEngine::GenerateGraph(std::string &dotScript, cDotEngine::eDotImageFormat format, std::string &resultImageFile)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	try
	{
	/*
		string::size_type idx = dotScript.find(" {");
		if (idx != string::npos)
		{
			string sname;
			sname = "digraph ";
			sname += sGUID;
			dotScript.erase(0,idx);
			dotScript.insert(0, sname);
		}
	*/
		string sFormat = "";
		switch (format)
		{
			case cDotEngine::DOT_GIF:
				sFormat = "gif";
				break;
			case cDotEngine::DOT_CMAPX:
				sFormat = "cmapx";
				break;
			case cDotEngine::DOT_JPG:
				sFormat = "jpg";
				break;
			case cDotEngine::DOT_PNG:
				sFormat = "png";
				break;
			case cDotEngine::DOT_WRL:
				sFormat = "wrl";
				break;
			case cDotEngine::DOT_SVG:
				sFormat = "svg";
				break;
		};
		if (sFormat.length() == 0)
		{
			resultImageFile = "";
			return HRC_DOTENGINE_INVALID_OUTPUT_FORMAT;
		}

		string sCmdline;
		sCmdline = m_dotpath;
#ifndef _PLATFORM_LINUX		
		sCmdline += "\\dot.exe";
#else
		sCmdline += "/dot";
#endif //! _PLATFORM_LINUX

		sCmdline += " -T";
		sCmdline += sFormat;

		sCmdline += " -o ";

		string sFileName;
		sFileName = m_WebDotFolder;
#ifndef _PLATFORM_LINUX		
		sFileName += "\\";
#else
		sFileName += "/";
#endif //! _PLATFORM_LINUX
		sFileName += resultImageFile;

		if (sFormat == "cmapx")
			m_MapFilePath = sFileName;
		else	
			m_MapFilePath = "";
		sCmdline += sFileName;

		int ret = m_ProcComm.Start(sCmdline.c_str(), m_dotpath.c_str());
		if (ret != HRC_PROCCOM_SUCCESS)
		{	
			return HRC_DOTENGINE_UNABLE_TO_LAUNCH_DOT;
		}
		ret = m_ProcComm.Send(dotScript.c_str(), dotScript.length());
		Sleep(2000);
		if (ret != HRC_PROCCOM_SUCCESS)
		{
			return HRC_DOTENGINE_UNABLE_TO_COMMUNICATE_WITH_DOT;
		}
		Sleep(2000);
		m_ProcComm.Stop();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return HRC_DOTENGINE_OK;
}


