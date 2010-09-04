/* mcscript-call-handler.h
   Copyright (C) 2005 Carlos Justiniano

mcscript-call-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-call-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-call-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-call-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_CALL_HANDLER_H
#define _MCSCRIPT_CALL_HANDLER_H

#include <string>
#include "mcscript.h"

/****************
 * Call Handler *
 ****************/
class cMCCallHandler : public cMCScriptKeywordHandler
{
public:
	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		cXMLLiteElement *pElm = pXML->GetElement();
		const char *pStatement = pElm->GetAttributeValue("statement");
		if (strlen(pStatement)==0)
			return HRC_MCSCRIPT_XMLDATA_CALLCMD_MISSING_STATEMENT_ATTRIBUTE;
		const char *pRetVar = pElm->GetAttributeValue("retvar");
		bool bHasRetVar = false;
		if (strlen(pRetVar)!=0)
			bHasRetVar = true;

		pOutputBuffer->append("\n");
		pOutputBuffer->append(pMCScript->GetIndentTabs());

		std::string sStatement = pStatement;
//		bool b = pMCScript->ExistsProcedure(sStatement);
//		if (b == false)
//			return HRC_MCSCRIPT_XMLDATA_CALLCMD_UNKNOWN_PROC;

		int ret = pMCScript->TranslateStatement(sStatement);
		if (ret == HRC_MCSCRIPT_OK)
		{		
			if (bHasRetVar)
			{
				pOutputBuffer->append(pMCScript->GetVariableDeclarations());
				pOutputBuffer->append("\n");
				pOutputBuffer->append(pMCScript->GetIndentTabs());
				pOutputBuffer->append("set ");
				pOutputBuffer->append(pRetVar+1);
				pOutputBuffer->append(" ");
				pOutputBuffer->append(sStatement);
			}
			else
			{
				pOutputBuffer->append(pMCScript->GetVariableDeclarations());
			}
		}
		return ret;
	}
};

#endif //_MCSCRIPT_CALL_HANDLER_H
