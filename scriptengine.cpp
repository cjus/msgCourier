/* scriptengine.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

scriptengine.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

scriptengine.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with scriptengine.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file scriptengine.cpp
 @brief TCL Scripting Engine wrapper
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
 TCL Scripting Engine wrapper
*/

#include <string>
#include <sstream>
#include <vector>
#include <time.h>

#include "tcl.h"
#include "cpptcl/cpptcl.h"

#include "log.h"
#include "exception.h"

#include "scriptengine.h"
#include "threadsync.h"
#include "core.h"
#include "url.h"

using namespace std;
using namespace Tcl;

class cTclInterpreter
{
public:
	static interpreter &GetInterpreter()
	{
		static interpreter m_interpreter;
		return m_interpreter;
	}
};

cThreadSync m_ThreadSync;
string m_sBuffer;

cCommandHandlerFactory *m_pCommandHandlerFactory;
cMsgQueue *m_pMsgQueue;
cSysMetrics *m_pSysMetrics;
cAccessLog *m_pAccessLog;

void _write(string s)
{
	try
	{	
		m_sBuffer += s;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}
/*
void _sendmsg(const object &obj)
{
	size_t elems;
	string s;
	char *p1, *p2;

	elems = obj.length(m_interpreter);
	for (size_t i=0; i<elems; i++)
	{
		object obj_name(obj.at(m_interpreter, i));
		object obj_value(obj.at(m_interpreter, ++i));
		p1 = (char*)(const char*)obj_name.get();
		p2 = (char*)(const char*)obj_value.get();
	}
}
*/

void _sendmsg(const object &obj)
{
	try
	{	
		interpreter interp(obj.get_interp(), false);

		size_t elems;
		string s;
		char *p1, *p2;

		elems = obj.length(interp);
		for (int i=0; i<(int)elems; i++)
		{
			object obj_name(obj.at(interp, i));
			object obj_value(obj.at(interp, ++i));
			p1 = (char*)(const char*)obj_name.get();
			p2 = (char*)(const char*)obj_value.get();
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	catch (...)
	{
		LOGALL("exception in _sendmsg");	
	}
}

string _getServerUptime()
{
	stringstream ss (stringstream::out);
	try
	{	
		ss << m_pSysMetrics->GetUptimeDays();
		ss << "d ";

		ss << m_pSysMetrics->GetUptimeHours();
		ss << "h ";

		ss << m_pSysMetrics->GetUptimeMinutes();
		ss << "m ";

		ss << m_pSysMetrics->GetUptimeSeconds();
		ss << "s";
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return ss.str();
}

int _getTotalMsgDelivered()
{
	return m_pSysMetrics->GetTotalMsgDelivered();
}

int _getTotalMsgUndelivered()
{
	return m_pSysMetrics->GetTotalMsgUndelivered();
}

int _getTotalMsgRejected()
{
	return m_pSysMetrics->GetTotalMsgRejected();
}

int _getTotalConAccepted()
{
	return m_pSysMetrics->GetTotalConAccepted();
}

int _getTotalConDropped()
{
	return m_pSysMetrics->GetTotalConDropped();
}

int _getTotalConRejected()
{
	return m_pSysMetrics->GetTotalConRejected();
}

string _getServerNodeName()
{
	return cCore::GetInstance()->GetServerName();
}

string _getServerNodeAddress()
{
	return cCore::GetInstance()->GetServerPrimaryAddr();
}

string  _getServerTCPPorts()
{
	return cCore::GetInstance()->GetTCPPorts();
}

string _getServerUDPPorts()
{
	return cCore::GetInstance()->GetUDPPorts();
}

string _isServerDatabaseActive()
{
	return "no";
}

object _getConnectionAcceptedGraphData()
{
    object tab;
	int indx,val;
	int arr[3601];
	m_pSysMetrics->GetConnectionAcceptedData(arr);
    for (indx = 0; indx < 3600; ++indx)
	{
		val = arr[indx];
		tab.append(cTclInterpreter::GetInterpreter(), object(val));
	}
	return tab;
}

object _getConnectionDroppedGraphData()
{
    object tab;
	int indx,val;
	int arr[3601];
	m_pSysMetrics->GetConnectionDroppedData(arr);
    for (indx = 0; indx < 3600; ++indx)
	{
		val = arr[indx];
		tab.append(cTclInterpreter::GetInterpreter(), object(val));
	}
	return tab;
}

object _getConnectionRejectedGraphData()
{
    object tab;
	int indx,val;
	int arr[3601];
	m_pSysMetrics->GetConnectionRejectedData(arr);
    for (indx = 0; indx < 3600; ++indx)
	{
		val = arr[indx];
		tab.append(cTclInterpreter::GetInterpreter(), object(val));
	}
	return tab;
}

object _getMessageDeliveredGraphData()
{
    object tab;
	int indx,val;
	int arr[3601];
	m_pSysMetrics->GetMessageDeliveredData(arr);
    for (indx = 0; indx < 3600; ++indx)
	{
		val = arr[indx];
		tab.append(cTclInterpreter::GetInterpreter(), object(val));
	}
	return tab;
}

object _getMessageUndeliveredGraphData()
{
    object tab;
	int indx,val;
	int arr[3601];
	m_pSysMetrics->GetMessageUndeliveredData(arr);
    for (indx = 0; indx < 3600; ++indx)
	{
		val = arr[indx];
		tab.append(cTclInterpreter::GetInterpreter(), object(val));
	}
	return tab;
}

object _getMessageRejectedGraphData()
{
    object tab;
	int indx,val;
	int arr[3601];
	m_pSysMetrics->GetMessageRejectedData(arr);
    for (indx = 0; indx < 3600; ++indx)
	{
		val = arr[indx];
		tab.append(cTclInterpreter::GetInterpreter(), object(val));
	}
	return tab;
}

string _getEventLog()
{
	string log;
	cLog::GetLogBuffer(log);
	return log;
}

object _getAccessLog()
{
	object tab;
	string s;
	vector<cAccessRecord> records;

	m_pAccessLog->GetEntries(records);
	int iSize = records.size();
	for (int i = 0; i < iSize; i++)
	{
		records[i].toString(s);
		tab.append(cTclInterpreter::GetInterpreter(), object(s));
	}
	return tab;
}

void _senddata(const object &obj)
{
	size_t size;
	const char *p = obj.get(size);
	FILE *fp = fopen("imagetest.gif", "wb");
	fwrite(p, size, 1, fp);
	fclose(fp);
}

string _mchandler(const object &obj)
{
	interpreter interp(obj.get_interp(), false);

	string sRet;
	char *pCmd, *pParam;

	size_t elems;
	elems = obj.length(interp);
	if (elems == 2)
	{
		object obj_name(obj.at(interp, 0));
		object obj_value(obj.at(interp, 1));
		pCmd = const_cast<char*>(obj_name.get());
		pParam = const_cast<char*>(obj_value.get());

		cICommandHandler *pHandler = m_pCommandHandlerFactory->GetCommandHandler(pCmd);
		if (pHandler)
			sRet = pHandler->OnProcessInternal(pParam);
	}

	return sRet;
}

string _urldecode(const object &obj)
{
	cURL url;
	interpreter interp(obj.get_interp(), false);

	string sRet;
	string sText;

	size_t elems;
	elems = obj.length(interp);
	if (elems == 1)
	{
		object obj_text(obj.at(interp, 0));
		sText = const_cast<char*>(obj_text.get());
		url.Decode(sText, sRet);
	}

	return sRet;
}

cScriptEngine::cScriptEngine()
{
//	string proc_send;
//	proc_send = "proc sendmsg amsg { upvar $amsg msg\n"
//				"_sendmsg [array get msg]\n"
//				"}\n";

	interpreter *pInterpreter = &cTclInterpreter::GetInterpreter();
	pInterpreter->make_safe();
//	pInterpreter->eval(proc_send);
	pInterpreter->def("_sendmsg", _sendmsg);

	pInterpreter->def("write", _write);
	pInterpreter->def("GetServerUptime", _getServerUptime);
	
	pInterpreter->def("GetTotalMsgDelivered", _getTotalMsgDelivered);
	pInterpreter->def("GetTotalMsgUndelivered", _getTotalMsgUndelivered);
	pInterpreter->def("GetTotalMsgRejected", _getTotalMsgRejected);

	pInterpreter->def("GetTotalConAccepted", _getTotalConAccepted);
	pInterpreter->def("GetTotalConDropped", _getTotalConDropped);
	pInterpreter->def("GetTotalConRejected", _getTotalConRejected);

	pInterpreter->def("GetServerNodeName", _getServerNodeName);
	pInterpreter->def("GetServerNodeAddress", _getServerNodeAddress);
	pInterpreter->def("GetServerTCPPorts", _getServerTCPPorts);
	pInterpreter->def("GetServerUDPPorts", _getServerUDPPorts);

	pInterpreter->def("IsServerDatabaseActive", _isServerDatabaseActive);

	pInterpreter->def("GetConnectionAcceptedGraphData", _getConnectionAcceptedGraphData);
	pInterpreter->def("GetConnectionDroppedGraphData", _getConnectionDroppedGraphData);
	pInterpreter->def("GetConnectionRejectedGraphData", _getConnectionRejectedGraphData);

	pInterpreter->def("GetMessageDeliveredGraphData", _getMessageDeliveredGraphData);
	pInterpreter->def("GetMessageUndeliveredGraphData", _getMessageUndeliveredGraphData);
	pInterpreter->def("GetMessageRejectedGraphData", _getMessageRejectedGraphData);

	pInterpreter->def("GetEventLog", _getEventLog);
	pInterpreter->def("GetAccessLog", _getAccessLog);

	pInterpreter->def("mc::senddata", _senddata);
	pInterpreter->def("mchandler", _mchandler);

	pInterpreter->def("urldecode", _urldecode);
}

cScriptEngine::~cScriptEngine()
{
}

void cScriptEngine::SetCommandHandlerFactory(cCommandHandlerFactory *pCommandHandlerFactory)
{
	m_pCommandHandlerFactory = pCommandHandlerFactory;
	::m_pCommandHandlerFactory = this->m_pCommandHandlerFactory;
}

void cScriptEngine::SetMsgQueue(cMsgQueue *pMsgQueue)
{
	m_pMsgQueue = pMsgQueue;
	::m_pMsgQueue = this->m_pMsgQueue;
}

void cScriptEngine::SetSysMetrics(cSysMetrics *pSysMetrics) 
{ 
	m_pSysMetrics = pSysMetrics; 
	::m_pSysMetrics = this->m_pSysMetrics;
}

void cScriptEngine::SetAccessLog(cAccessLog *pAccessLog) 
{ 
	m_pAccessLog = pAccessLog; 
	::m_pAccessLog = this->m_pAccessLog;
}

bool cScriptEngine::Execute(string &sNamespace, string &sScriptInput, string &sScriptOutput, bool silent)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	bool bRet = true;
	try
	{
		string sScript;

		///*
		sScript = "namespace eval ";
		sScript += sNamespace;
		sScript += " {\n";
		sScript += sScriptInput;
		sScript += "\n}";
		//*/
		//sScript = sScriptInput;

		m_sBuffer = "";
		if (silent)
		{
			cTclInterpreter::GetInterpreter().eval(sScript);
		}
		else
		{
			m_sBuffer.append(cTclInterpreter::GetInterpreter().eval(sScript));		
		}
	}
	catch (tcl_error const &e)
	{
		m_sBuffer = e.what();
		LOGALL(e.what());
		bRet = false;
	}
	catch (exception const &e)
	{
		m_sBuffer = e.what();
		LOGALL(e.what());
		bRet = false;
	}
	catch (...)
	{
		m_sBuffer = "Error: unknown";
		LOGALL(m_sBuffer.c_str());
		bRet = false;
	}

	sScriptOutput = m_sBuffer;
	return bRet;
}

void cScriptEngine::DeleteNamespace(string &sNamespace, string &sScriptOutput)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	string sCmd;
	sCmd = "namespace delete ";
	sCmd += sNamespace.c_str();

	m_sBuffer = "";

	try
	{
		cTclInterpreter::GetInterpreter().eval(sCmd);
	}
	catch (tcl_error const &e)
	{
		m_sBuffer = e.what();
		LOGALL(e.what());
	}
	catch (exception const &e)
	{
		m_sBuffer = e.what();
		LOGALL(e.what());
	}
	catch (...)
	{
		m_sBuffer = "Error: unknown";
		LOGALL(m_sBuffer.c_str());
	}

	sScriptOutput = m_sBuffer;
}
