/* mcscript-pair-handler.h
   Copyright (C) 2006 Carlos Justiniano

mcscript-pair-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-pair-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-pair-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-pair-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_PAIR_HANDLER_H
#define _MCSCRIPT_PAIR_HANDLER_H

#include <string>
#include "mcscript.h"

/****************
 * Pair Handler *
 ***************/
class cMCPairHandler : public cMCScriptKeywordHandler
{
public:
	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		cXMLLiteElement *pElm = pXML->GetElement();

		const char *pKey = pElm->GetAttributeValue("key");
		if (strlen(pKey)==0)
			return HRC_MCSCRIPT_XMLDATA_PAIRCMD_MISSING_KEY_ATTRIBUTE;
		const char *pValue = pElm->GetAttributeValue("value");
		if (strlen(pValue)==0)
			return HRC_MCSCRIPT_XMLDATA_PAIRCMD_MISSING_VALUE_ATTRIBUTE;

		pOutputBuffer->append("\n");
/*
		std::string sStatement = pValue;
		int ret = pMCScript->TranslateStatement(sStatement);
		if (ret == HRC_MCSCRIPT_OK)
		{
			pOutputBuffer->insert(pMCScript->GetMapMarker(), pMCScript->GetVariableDeclarations());

			pOutputBuffer->append(pMCScript->GetIndentTabs());
			pOutputBuffer->append("{");
			pOutputBuffer->append(pKey);
			pOutputBuffer->append("} ");
			pOutputBuffer->append(sStatement);
			pOutputBuffer->append("");
		}
		else
		{
*/
			pOutputBuffer->append(pMCScript->GetIndentTabs());
			pOutputBuffer->append("{");
			pOutputBuffer->append(pKey);
			pOutputBuffer->append("} {");
			pOutputBuffer->append(pValue);
			pOutputBuffer->append("}");
/*
		}
*/
		return 0;
	}
};

#endif //_MCSCRIPT_PAIR_HANDLER_H
