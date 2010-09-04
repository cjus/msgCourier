/* scriptengine.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

scriptengine.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

scriptengine.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with scriptengine.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file scriptengine.h
 @brief TCL Scripting Engine wrapper
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
 TCL Scripting Engine wrapper
*/

#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <string>

#include "msgqueue.h"
#include "imsg.h"
#include "commandhandlerfactory.h"
#include "sysmetrics.h"
#include "accesslog.h"
#include "scriptenginebase.h"

class cScriptEngine : public cScriptEngineBase
{
public:
	cScriptEngine();
	~cScriptEngine();

	void SetCommandHandlerFactory(cCommandHandlerFactory *pCommandHandlerFactory);
	void SetMsgQueue(cMsgQueue *pMsgQueue);
	void SetSysMetrics(cSysMetrics *pSysMetrics);
	void SetAccessLog(cAccessLog *pAccessLog);

	bool Execute(std::string &sNamespace, std::string &sScriptInput, std::string &sScriptOutput, bool silent);
	void DeleteNamespace(std::string &sNamespace, std::string &sScriptOutput);
private:
	cCommandHandlerFactory *m_pCommandHandlerFactory;
	cMsgQueue *m_pMsgQueue;
	cSysMetrics *m_pSysMetrics;
	cAccessLog *m_pAccessLog;
};

#endif // SCRIPTENGINE_H

