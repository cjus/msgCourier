/* mcscript-list-handler.h
   Copyright (C) 2005 Carlos Justiniano

mcscript-list-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-list-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-list-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-list-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_LIST_HANDLER_H
#define _MCSCRIPT_LIST_HANDLER_H

#include <string>
#include "mcscript.h"

/****************
 * List Handler *
 ****************/
class cMCListHandler : public cMCScriptKeywordHandler
{
public:
	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		const char *pData;
		cXMLLiteElement *pElm = pXML->GetElement();
		const char *pVarName = pElm->GetAttributeValue("name");
		if (strlen(pVarName)==0)
			return HRC_MCSCRIPT_XMLDATA_LISTCMD_MISSING_NAME_ATTRIBUTE;

		pOutputBuffer->append("\n");
		pOutputBuffer->append(pMCScript->GetIndentTabs());

		pOutputBuffer->append("set ");
		pOutputBuffer->append(pVarName);
		pOutputBuffer->append(" [list ");
		pXML->MoveNext();
		do
		{
			pElm = pXML->GetElement();
			pData = const_cast<char*>(pElm->GetElementName());
			if (strcmp(pData, "item") != 0 && strcmp(pData, "/item") !=0)
				break;
			if (strcmp(pData, "/item") !=0)
			{
				pData = pElm->GetElementValue();
				pOutputBuffer->append(" '");
				pOutputBuffer->append(pData);
				pOutputBuffer->append("'");
			}
		} while (pXML->MoveNext() == HRC_XMLLITELIST_OK);
		pOutputBuffer->append("]");
		return 0;
	}
};

#endif //_MCSCRIPT_LIST_HANDLER_H
