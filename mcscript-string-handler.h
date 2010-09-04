/* mcscript-string-handler.h
   Copyright (C) 2005 Carlos Justiniano

mcscript-string-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-string-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-string-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-string-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_STRING_HANDLER_H
#define _MCSCRIPT_STRING_HANDLER_H

#include <string>
#include "mcscript.h"

/******************
 * String Handler *
 ******************/
class cMCStringHandler : public cMCScriptKeywordHandler
{
public:
	void AppendMatchCase(std::string *pOutputBuffer, const char *pMatchCase)
	{
		if (strlen(pMatchCase)!=0 && strcmp(pMatchCase, "no")==0)
			pOutputBuffer->append("-nocase ");
	}

	void AppendLength(std::string *pOutputBuffer, const char *pLength)
	{
		if (strlen(pLength)!=0)
		{
			pOutputBuffer->append("-length ");
			pOutputBuffer->append(pLength);
			pOutputBuffer->append(" ");
		}
	}

	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		cXMLLiteElement *pElm = pXML->GetElement();
		const char *pRetVar = pElm->GetAttributeValue("retvar");
		if (strlen(pRetVar)==0)
			return HRC_MCSCRIPT_XMLDATA_STRINGCMD_MISSING_RETVAR_ATTRIBUTE;
		if (pRetVar[0] != '$')
			return HRC_MCSCRIPT_XMLDATA_STRINGCMD_RETVAR_NOT_DOLLAR_PREFIXED;

		const char *pOperation = pElm->GetAttributeValue("operation");
		if (strlen(pOperation)==0)
			return HRC_MCSCRIPT_XMLDATA_STRINGCMD_MISSING_OPERATION_ATTRIBUTE;

		const char *pMatchCase = pElm->GetAttributeValue("matchcase");
		if (strlen(pMatchCase)!=0 && (strcmp(pMatchCase,"yes")!=0 && strcmp(pMatchCase,"no")!=0))
			return HRC_MCSCRIPT_XMLDATA_STRINGCMD_MATCHCASE_ATTRIBUTE_NOT_YESNO;

		const char *pVar = pElm->GetAttributeValue("var");
		const char *pVar1 = pElm->GetAttributeValue("var1");
		const char *pVar2 = pElm->GetAttributeValue("var2");
		const char *pLen = pElm->GetAttributeValue("len");
		const char *pStart = pElm->GetAttributeValue("start");
		const char *pEnd = pElm->GetAttributeValue("end");
		const char *pWith = pElm->GetAttributeValue("with");
		const char *pChars = pElm->GetAttributeValue("chars");
		const char *pPattern = pElm->GetAttributeValue("pattern");

		std::string sOperation = pOperation;
		if (sOperation == "compare")
		{
			if (strlen(pVar1)==0 || strlen(pVar2)==0)
				return HRC_MCSCRIPT_XMLDATA_STRINGCMD_COMPARE_ATTRIBUTE_REQUIRES_VAR1VAR2;
			pOutputBuffer->append("\n");
			pOutputBuffer->append(pMCScript->GetIndentTabs());

			pOutputBuffer->append("set ");
			pOutputBuffer->append(pRetVar+1);
			pOutputBuffer->append(" [string compare ");
			AppendMatchCase(pOutputBuffer, pMatchCase);
			AppendLength(pOutputBuffer, pLen);
			pOutputBuffer->append(pVar1);
			pOutputBuffer->append(" ");
			pOutputBuffer->append(pVar2);
			pOutputBuffer->append("]");
		}
		else if (sOperation == "copy")
		{
			if (strlen(pVar)==0)
				return HRC_MCSCRIPT_XMLDATA_STRINGCMD_COPY_REQUIRES_VAR_ATTRIBUTE;
			if (strlen(pStart)==0)
				pStart = "0";
			if (strlen(pEnd)==0)
				pEnd = "end";
			pOutputBuffer->append("\n");
			pOutputBuffer->append(pMCScript->GetIndentTabs());

			pOutputBuffer->append("set ");
			pOutputBuffer->append(pRetVar+1);
			pOutputBuffer->append(" [string range ");
			pOutputBuffer->append(pVar);
			pOutputBuffer->append(" ");
			pOutputBuffer->append(pStart);
			pOutputBuffer->append(" ");
			pOutputBuffer->append(pEnd);
			pOutputBuffer->append("]");
		}
		else if (sOperation == "length")
		{
			if (strlen(pVar)==0 && (strlen(pVar1)!=0 || strlen(pVar2)!=0))
				return HRC_MCSCRIPT_XMLDATA_STRINGCMD_LENGTH_ATTRIBUTE_DOESNT_USE_VAR1VAR2;
			if (strlen(pVar)==0)
				return HRC_MCSCRIPT_XMLDATA_STRINGCMD_LENGTH_REQUIRES_VAR_ATTRIBUTE;

			pOutputBuffer->append("\n");
			pOutputBuffer->append(pMCScript->GetIndentTabs());

			pOutputBuffer->append("set ");
			pOutputBuffer->append(pRetVar+1);
			pOutputBuffer->append(" [string length ");
			pOutputBuffer->append(pVar);
			pOutputBuffer->append("]");
		}
		else if (sOperation == "match")
		{
			if (strlen(pVar)==0)
				return HRC_MCSCRIPT_XMLDATA_STRINGCMD_MATCH_REQUIRES_VAR_ATTRIBUTE;
			if (strlen(pPattern)==0)
				return HRC_MCSCRIPT_XMLDATA_STRINGCMD_MATCH_REQUIRES_PATTERN_ATTRIBUTE;
			pOutputBuffer->append("\n");
			pOutputBuffer->append(pMCScript->GetIndentTabs());

			pOutputBuffer->append("set ");
			pOutputBuffer->append(pRetVar+1);
			pOutputBuffer->append(" [string match ");
			AppendMatchCase(pOutputBuffer, pMatchCase);
			pOutputBuffer->append("{");
			pOutputBuffer->append(pPattern);
			pOutputBuffer->append("} ");
			pOutputBuffer->append(pVar);
			pOutputBuffer->append("]");
		}
		else if (sOperation == "replace")
		{
			if (strlen(pVar)==0)
				return HRC_MCSCRIPT_XMLDATA_STRINGCMD_REPLACE_REQUIRES_VAR_ATTRIBUTE;
			if (strlen(pStart)==0)
				pStart = "0";
			if (strlen(pEnd)==0)
				pEnd = "end";

			pOutputBuffer->append("\n");
			pOutputBuffer->append(pMCScript->GetIndentTabs());

			pOutputBuffer->append("set ");
			pOutputBuffer->append(pRetVar+1);
			pOutputBuffer->append(" [string replace ");
			pOutputBuffer->append(pVar);
			pOutputBuffer->append(" ");
			pOutputBuffer->append(pStart);
			pOutputBuffer->append(" ");
			pOutputBuffer->append(pEnd);
			pOutputBuffer->append(" {");
			pOutputBuffer->append(pWith);
			pOutputBuffer->append("}]");
		}
		else if (sOperation == "tolower")
		{
			if (strlen(pVar)==0)
				return HRC_MCSCRIPT_XMLDATA_STRINGCMD_TOLOWER_REQUIRES_VAR_ATTRIBUTE;
			if (strlen(pStart)==0)
				pStart = "0";
			if (strlen(pEnd)==0)
				pEnd = "end";

			pOutputBuffer->append("\n");
			pOutputBuffer->append(pMCScript->GetIndentTabs());

			pOutputBuffer->append("set ");
			pOutputBuffer->append(pRetVar+1);
			pOutputBuffer->append(" [string tolower ");
			pOutputBuffer->append(pVar);
			pOutputBuffer->append(" ");
			pOutputBuffer->append(pStart);
			pOutputBuffer->append(" ");
			pOutputBuffer->append(pEnd);
			pOutputBuffer->append("]");
		}
		else if (sOperation == "toupper")
		{
			if (strlen(pVar)==0)
				return HRC_MCSCRIPT_XMLDATA_STRINGCMD_TOUPPER_REQUIRES_VAR_ATTRIBUTE;
			if (strlen(pStart)==0)
				pStart = "0";
			if (strlen(pEnd)==0)
				pEnd = "end";

			pOutputBuffer->append("\n");
			pOutputBuffer->append(pMCScript->GetIndentTabs());

			pOutputBuffer->append("set ");
			pOutputBuffer->append(pRetVar+1);
			pOutputBuffer->append(" [string toupper ");
			pOutputBuffer->append(pVar);
			pOutputBuffer->append(" ");
			pOutputBuffer->append(pStart);
			pOutputBuffer->append(" ");
			pOutputBuffer->append(pEnd);
			pOutputBuffer->append("]");
		}
		else if (sOperation == "trimleft")
		{
			if (strlen(pVar)==0)
				return HRC_MCSCRIPT_XMLDATA_STRINGCMD_TRIMLEFT_REQUIRES_VAR_ATTRIBUTE;

			pOutputBuffer->append("\n");
			pOutputBuffer->append(pMCScript->GetIndentTabs());

			pOutputBuffer->append("set ");
			pOutputBuffer->append(pRetVar+1);
			pOutputBuffer->append(" [string trimleft ");
			pOutputBuffer->append(pVar);
			if (strlen(pChars)!=0)
			{
				pOutputBuffer->append(" \"");
				pOutputBuffer->append(pChars);
				pOutputBuffer->append("\"");
			}
			pOutputBuffer->append("]");
		}
		else if (sOperation == "trimright")
		{
			if (strlen(pVar)==0)
				return HRC_MCSCRIPT_XMLDATA_STRINGCMD_TRIMRIGHT_REQUIRES_VAR_ATTRIBUTE;
			pOutputBuffer->append("\n");
			pOutputBuffer->append(pMCScript->GetIndentTabs());
			pOutputBuffer->append("set ");
			pOutputBuffer->append(pRetVar+1);
			pOutputBuffer->append(" [string trimright ");
			pOutputBuffer->append(pVar);
			if (strlen(pChars)!=0)
			{
				pOutputBuffer->append(" \"");
				pOutputBuffer->append(pChars);
				pOutputBuffer->append("\"");
			}
			pOutputBuffer->append("]");
		}
		else
		{
			return HRC_MCSCRIPT_XMLDATA_STRINGCMD_UNKNOWN_OPERATION_ATTRIBUTE;
		}
		return 0;
	}
};

#endif //_MCSCRIPT_STRING_HANDLER_H
