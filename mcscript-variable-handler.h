/* mcscript-variable-handler.h
   Copyright (C) 2005 Carlos Justiniano

mcscript-variable-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-variable-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-variable-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-variable-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_VARIABLE_HANDLER_H
#define _MCSCRIPT_VARIABLE_HANDLER_H

#include <string>
#include "mcscript.h"

/********************
 * Variable Handler *
 ********************/
class cMCVariableHandler : public cMCScriptKeywordHandler
{
public:
	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		cXMLLiteElement *pElm = pXML->GetElement();
		const char *pVarName = pElm->GetAttributeValue("name");
		if (strlen(pVarName)==0)
			return HRC_MCSCRIPT_XMLDATA_VARIABLECMD_MISSING_NAME_ATTRIBUTE;

		const char *pValue = pElm->GetAttributeValue("value");
		const char *pExpr = pElm->GetAttributeValue("expr");

		bool bHasValueAttribute = (strlen(pValue)!=0) ? true : false;
		bool bHasExprAttribute = (strlen(pExpr)!=0) ? true : false;
		if (bHasValueAttribute == false && bHasExprAttribute == false)
		{
			bHasValueAttribute = true;
		}
		if (bHasValueAttribute && bHasExprAttribute)
			return HRC_MCSCRIPT_XMLDATA_VARIABLECMD_INCLUDES_VALUEANDEXPR_ATTRIBUTE;

		if (bHasValueAttribute)
		{
			std::string sStatement = pValue;
			int ret = pMCScript->TranslateStatement(sStatement);
			if (ret == HRC_MCSCRIPT_OK)
			{
				//pOutputBuffer->append("\n");
				//pOutputBuffer->append(pMCScript->GetIndentTabs());

				pOutputBuffer->append(pMCScript->GetVariableDeclarations());
				pOutputBuffer->append(pMCScript->GetIndentTabs());
				pOutputBuffer->append("set ");
				pOutputBuffer->append(pVarName);
				pOutputBuffer->append(" ");
				pOutputBuffer->append("\"");
				pOutputBuffer->append(sStatement);
				pOutputBuffer->append("\"");
			}
			else
			{
				pOutputBuffer->append("\n");
				pOutputBuffer->append(pMCScript->GetIndentTabs());
				pOutputBuffer->append("set ");
				pOutputBuffer->append(pVarName);
				pOutputBuffer->append(" ");
				pOutputBuffer->append("\"");
				pOutputBuffer->append(pValue);
				pOutputBuffer->append("\"");
			}
		}
		else if (bHasExprAttribute)
		{
			pOutputBuffer->append("\n");
			pOutputBuffer->append(pMCScript->GetIndentTabs());

			pOutputBuffer->append("set ");
			pOutputBuffer->append(pVarName);
			pOutputBuffer->append(" [expr { ");
			pOutputBuffer->append(pExpr);
			pOutputBuffer->append(" }]");
		}
		return 0;
	}
};

#endif //_MCSCRIPT_VARIABLE_HANDLER_H
