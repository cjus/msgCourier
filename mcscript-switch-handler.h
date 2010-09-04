/* mcscript-switch-handler.h
   Copyright (C) 2005 Carlos Justiniano

mcscript-switch-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-switch-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-switch-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-switch-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_SWITCH_HANDLER_H
#define _MCSCRIPT_SWITCH_HANDLER_H

#include <string>
#include "mcscript.h"

/******************
 * Switch Handler *
 *****************/
class cMCSwitchHandler : public cMCScriptKeywordHandler
{
public:
	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		cXMLLiteElement *pElm = pXML->GetElement();

		const char *pOperation = pElm->GetAttributeValue("operation");
		if (strlen(pOperation)==0)
			return HRC_MCSCRIPT_XMLDATA_SWITCHCMD_MISSING_OPERATION_ATTRIBUTE;
		const char *pVariable = pElm->GetAttributeValue("var");
		if (strlen(pVariable)==0)
			return HRC_MCSCRIPT_XMLDATA_SWITCHCMD_MISSING_VAR_ATTRIBUTE;

		if (strcmp(pOperation, "exact") != 0 &&
			strcmp(pOperation, "glob") != 0 &&
			strcmp(pOperation, "regexp") != 0)
		{
			return HRC_MCSCRIPT_XMLDATA_SWITCHCMD_INVALID_OPERATION;
		}
		if (pVariable[0] != '$')
			return HRC_MCSCRIPT_XMLDATA_SWITCHCMD_VAR_NOT_DOLLAR_PREFIX;
		if (strstr(pVariable, "[") != 0)
			return HRC_MCSCRIPT_XMLDATA_SWITCHCMD_VAR_MAYNOT_USE_ARRAY;

		pOutputBuffer->append("\n");
		pOutputBuffer->append(pMCScript->GetIndentTabs());

		pOutputBuffer->append("switch -");
		pOutputBuffer->append(pOperation);
		pOutputBuffer->append(" ");
		pOutputBuffer->append(pVariable);
		pOutputBuffer->append(" {");
		pMCScript->IncreaseIndentDepth();

		return 0;
	}
};

#endif //_MCSCRIPT_SWITCH_HANDLER_H
