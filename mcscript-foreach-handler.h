/* mcscript-foreach-handler.h
   Copyright (C) 2005 Carlos Justiniano

mcscript-foreach-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-foreach-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-foreach-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-foreach-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_FOREACH_HANDLER_H
#define _MCSCRIPT_FOREACH_HANDLER_H

#include <string>
#include "mcscript.h"

/*******************
 * Foreach Handler *
 *******************/
class cMCForeachHandler : public cMCScriptKeywordHandler
{
public:
	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		cXMLLiteElement *pElm = pXML->GetElement();

		const char *pVariable = pElm->GetAttributeValue("var");
		if (strlen(pVariable)==0)
			return HRC_MCSCRIPT_XMLDATA_FOREACHCMD_MISSING_VAR_ATTRIBUTE;

		const char *pList = pElm->GetAttributeValue("list");
		if (strlen(pList)==0)
			return HRC_MCSCRIPT_XMLDATA_FOREACHCMD_MISSING_LIST_ATTRIBUTE;

		if (pVariable[0] != '$')
			return HRC_MCSCRIPT_XMLDATA_FOREACHCMD_VAR_NOT_DOLLAR_PREFIX;
		if (strstr(pVariable, "[") != 0)
			return HRC_MCSCRIPT_XMLDATA_FOREACHCMD_VAR_MAYNOT_USE_ARRAY;

		if (pList[0] != '$')
			return HRC_MCSCRIPT_XMLDATA_FOREACHCMD_TEST_NOT_DOLLAR_PREFIX;
		if (strstr(pList, "[") != 0)
			return HRC_MCSCRIPT_XMLDATA_FOREACHCMD_TEST_MAYNOT_USE_ARRAY;

		pOutputBuffer->append("\n");
		pOutputBuffer->append(pMCScript->GetIndentTabs());
		pOutputBuffer->append("set ");
		pOutputBuffer->append(pVariable+1);
		pOutputBuffer->append(" 0");

		pOutputBuffer->append("\n");
		pOutputBuffer->append(pMCScript->GetIndentTabs());

		pOutputBuffer->append("foreach ");
		pOutputBuffer->append(pVariable+1);
		pOutputBuffer->append(" ");
		pOutputBuffer->append(pList);
		pOutputBuffer->append(" {");
		pMCScript->IncreaseIndentDepth();
		return 0;
	}
};

#endif //_MCSCRIPT_FOREACH_HANDLER_H
