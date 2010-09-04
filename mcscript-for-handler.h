/* mcscript-for-handler.h
   Copyright (C) 2005 Carlos Justiniano

mcscript-for-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-for-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-for-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-for-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_FOR_HANDLER_H
#define _MCSCRIPT_FOR_HANDLER_H

#include <string>
#include "mcscript.h"

/***************
 * For Handler *
 ***************/
class cMCForHandler : public cMCScriptKeywordHandler
{
public:
	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		cXMLLiteElement *pElm = pXML->GetElement();

		const char *pStartExpression = pElm->GetAttributeValue("start");
		if (strlen(pStartExpression)==0)
			return HRC_MCSCRIPT_XMLDATA_FORCMD_MISSING_START_ATTRIBUTE;
		const char *pTestExpression = pElm->GetAttributeValue("test");
		if (strlen(pTestExpression)==0)
			return HRC_MCSCRIPT_XMLDATA_FORCMD_MISSING_TEST_ATTRIBUTE;
		const char *pIncrbyExpression = pElm->GetAttributeValue("incr");
		if (strlen(pIncrbyExpression)==0)
			return HRC_MCSCRIPT_XMLDATA_FORCMD_MISSING_INCR_ATTRIBUTE;

		if (pStartExpression[0] != '$')
			return HRC_MCSCRIPT_XMLDATA_FORCMD_START_NOT_DOLLAR_PREFIX;
		if (strstr(pStartExpression, "[") != 0)
			return HRC_MCSCRIPT_XMLDATA_FORCMD_START_MAYNOT_USE_ARRAY;

		if (pTestExpression[0] != '$')
			return HRC_MCSCRIPT_XMLDATA_FORCMD_TEST_NOT_DOLLAR_PREFIX;
		if (strstr(pTestExpression, "[") != 0)
			return HRC_MCSCRIPT_XMLDATA_FORCMD_TEST_MAYNOT_USE_ARRAY;


		std::string sVar;
		std::string sValue;
			std::string sStartExpression = pStartExpression;
			
		if (pMCScript->GetVarValue(sStartExpression, sVar, sValue))
		{
			pOutputBuffer->append("\n");
			pOutputBuffer->append(pMCScript->GetIndentTabs());

			pOutputBuffer->append("for {set ");
			pOutputBuffer->append(sVar);
			pOutputBuffer->append(" ");
			pOutputBuffer->append(sValue);
			pOutputBuffer->append("} {");
			pOutputBuffer->append(pTestExpression);
			pOutputBuffer->append("} {incr ");
			pOutputBuffer->append(sVar);
			pOutputBuffer->append(" ");
			pOutputBuffer->append(pIncrbyExpression);
			pOutputBuffer->append("} {");
			pMCScript->IncreaseIndentDepth();
		}
		else
		{
		}

		return 0;
	}
};

#endif //_MCSCRIPT_FOR_HANDLER_H
