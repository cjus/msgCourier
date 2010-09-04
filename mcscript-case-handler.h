/* mcscript-case-handler.h
   Copyright (C) 2005 Carlos Justiniano

mcscript-case-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-case-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-case-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-case-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_CASE_HANDLER_H
#define _MCSCRIPT_CASE_HANDLER_H

#include <string>
#include "mcscript.h"

/***************
 * Case Handler *
 ***************/
class cMCCaseHandler : public cMCScriptKeywordHandler
{
	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		cXMLLiteElement *pElm = pXML->GetElement();

		const char *pPattern = pElm->GetAttributeValue("pattern");
		if (strlen(pPattern)==0)
			return HRC_MCSCRIPT_XMLDATA_SWITCHCMD_MISSING_PATTERN_ATTRIBUTE;
		const char *pFallthrough = pElm->GetAttributeValue("fallthrough");
		if (strlen(pFallthrough)==0)
			return HRC_MCSCRIPT_XMLDATA_SWITCHCMD_MISSING_FALLTHROUGH_ATTRIBUTE;

		if (strcmp(pFallthrough, "true") != 0 &&
			strcmp(pFallthrough, "false") != 0)
		{
			return HRC_MCSCRIPT_XMLDATA_SWITCHCMD_FALLTHROUGH_MUSTBE_TRUE_OR_FALSE;
		}

		pOutputBuffer->append("\n");
		pOutputBuffer->append(pMCScript->GetIndentTabs());

		pOutputBuffer->append("\"");
		pOutputBuffer->append(pPattern);
		if (strcmp(pFallthrough,"false")==0)
		{
			pOutputBuffer->append("\" {");
			pMCScript->IncreaseIndentDepth();
		}
		else
		{
			pOutputBuffer->append("\" -");
		}
		return 0;
	}
};

#endif //_MCSCRIPT_CASE_HANDLER_H
