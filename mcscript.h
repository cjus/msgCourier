/* mcscript.h
   Copyright (C) 2005 Carlos Justiniano

mcscript.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript.h
 @brief Message Courier Script
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script translates an XML script to a TCL implementation.
*/

#ifndef _MCSCRIPT_H
#define _MCSCRIPT_H

#include <string>
#include <map>
#include <stack>
#include <queue>

#include "xmlliteparser.h"

#define HRC_MCSCRIPT_OK													0x0000
#define HRC_MCSCRIPT_FILE_ERROR											0x0001
#define HRC_MCSCRIPT_MEMORY_ERROR										0x0002
#define HRC_MCSCRIPT_XMLDATA_EMPTY										0x0003
#define HRC_MCSCRIPT_XMLDATA_NEEDLOWERCASE_ELEMENT						0x0004
#define HRC_MCSCRIPT_XMLDATA_NEEDLOWERCASE_ATTRIBUTE					0x0005
#define HRC_MCSCRIPT_XMLDATA_INVALIDROOT								0x0006
#define HRC_MCSCRIPT_XMLDATA_INVALIDTAGS								0x0007
#define HRC_MCSCRIPT_XMLDATA_OPENCLOSE_TAGMISMATCH						0x0008
#define HRC_MCSCRIPT_XMLDATA_MISSING_TAG_DURING_TRANSLATE				0x0009
#define HRC_MCSCRIPT_XMLDATA_OUTPUTCMD_MISSING_TEXT_ATTRIBUTE			0x000A
#define HRC_MCSCRIPT_INIT_FAILED										0x000B
#define HRC_MCSCRIPT_XMLDATA_VARIABLECMD_MISSING_NAME_ATTRIBUTE			0x000C
#define HRC_MCSCRIPT_XMLDATA_VARIABLECMD_INCLUDES_VALUEANDEXPR_ATTRIBUTE 0x000D
#define HRC_MCSCRIPT_XMLDATA_VARIABLECMD_MISSING_VALUEOREXPR_ATTRIBUTE	0x000E
#define HRC_MCSCRIPT_XMLDATA_IFCMD_MISSING_TEST_ATTRIBUTE				0x000F
#define HRC_MCSCRIPT_XMLDATA_INCRCMD_MISSING_VARIABLE_ATTRIBUTE			0x0010
#define HRC_MCSCRIPT_XMLDATA_INCRCMD_MISSING_BY_ATTRIBUTE				0x0011
#define HRC_MCSCRIPT_XMLDATA_WHILECMD_MISSING_TEST_ATTRIBUTE			0x0012
#define HRC_MCSCRIPT_XMLDATA_LISTCMD_MISSING_NAME_ATTRIBUTE				0x0013
#define HRC_MCSCRIPT_XMLDATA_SCRIPTCMD_MISSING_LANGUAGE_ATTRIBUTE		0x0014
#define HRC_MCSCRIPT_XMLDATA_SCRIPTCMD_LANGUAGE_MUSTBE_TCL				0x0015
#define HRC_MCSCRIPT_XMLDATA_SCRIPTCMD_ISMISSING_CDATA_SECTION			0x0016
#define HRC_MCSCRIPT_XMLDATA_IMPORTCMD_MISSING_FILE_ATTRIBUTE			0x0017
#define HRC_MCSCRIPT_XMLDATA_IMPORTCMD_FILELOAD_FAILED					0x0018
#define HRC_MCSCRIPT_XMLDATA_CALLCMD_MISSING_STATEMENT_ATTRIBUTE		0x0019
#define HRC_MCSCRIPT_XMLDATA_CALLCMD_UNKNOWN_PROC						0x0020
#define HRC_MCSCRIPT_XMLDATA_PROCEDURECMD_MISSING_PROCNAME_ATTRIBUTE	0x0021
#define HRC_MCSCRIPT_XMLDATA_PROCEDURECMD_MISSING_PARAMNAME_ATTRIBUTE	0x0022
#define HRC_MCSCRIPT_XMLDATA_RETURNCMD_MISSING_VALUE_ATTRIBUTE			0x0023
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_MISSING_RETVAR_ATTRIBUTE			0x0024
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_MISSING_OPERATION_ATTRIBUTE		0x0025
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_UNKNOWN_OPERATION_ATTRIBUTE		0x0026
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_MATCHCASE_ATTRIBUTE_NOT_YESNO	0x0027
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_COMPARE_ATTRIBUTE_REQUIRES_VAR1VAR2	0x0028
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_RETVAR_NOT_DOLLAR_PREFIXED		0x0029
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_LENGTH_ATTRIBUTE_DOESNT_USE_VAR1VAR2	0x0030
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_LENGTH_REQUIRES_VAR_ATTRIBUTE	0x0031
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_MATCH_REQUIRES_VAR_ATTRIBUTE		0x0032
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_MATCH_REQUIRES_PATTERN_ATTRIBUTE	0x0033
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_COPY_REQUIRES_VAR_ATTRIBUTE		0x0034
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_REPLACE_REQUIRES_VAR_ATTRIBUTE	0x0035
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_TOLOWER_REQUIRES_VAR_ATTRIBUTE	0x0036
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_TOUPPER_REQUIRES_VAR_ATTRIBUTE	0x0037
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_TRIMLEFT_REQUIRES_VAR_ATTRIBUTE	0x0038
#define HRC_MCSCRIPT_XMLDATA_STRINGCMD_TRIMRIGHT_REQUIRES_VAR_ATTRIBUTE	0x0039

#define HRC_MCSCRIPT_XMLDATA_FORCMD_MISSING_START_ATTRIBUTE				0x0040
#define HRC_MCSCRIPT_XMLDATA_FORCMD_MISSING_TEST_ATTRIBUTE				0x0041
#define HRC_MCSCRIPT_XMLDATA_FORCMD_MISSING_INCR_ATTRIBUTE				0x0042
#define HRC_MCSCRIPT_XMLDATA_FORCMD_START_NOT_DOLLAR_PREFIX				0x0043
#define HRC_MCSCRIPT_XMLDATA_FORCMD_START_MAYNOT_USE_ARRAY				0x0044
#define HRC_MCSCRIPT_XMLDATA_FORCMD_TEST_MAYNOT_USE_ARRAY				0x0045
#define HRC_MCSCRIPT_XMLDATA_FORCMD_TEST_NOT_DOLLAR_PREFIX				0x0046

#define HRC_MCSCRIPT_XMLDATA_FOREACHCMD_MISSING_VAR_ATTRIBUTE			0x0047
#define HRC_MCSCRIPT_XMLDATA_FOREACHCMD_MISSING_LIST_ATTRIBUTE			0x0048
#define HRC_MCSCRIPT_XMLDATA_FOREACHCMD_VAR_NOT_DOLLAR_PREFIX			0x0049
#define HRC_MCSCRIPT_XMLDATA_FOREACHCMD_VAR_MAYNOT_USE_ARRAY			0x0050
#define HRC_MCSCRIPT_XMLDATA_FOREACHCMD_TEST_NOT_DOLLAR_PREFIX			0x0051
#define HRC_MCSCRIPT_XMLDATA_FOREACHCMD_TEST_MAYNOT_USE_ARRAY			0x0052

#define HRC_MCSCRIPT_XMLDATA_SWITCHCMD_MISSING_PATTERN_ATTRIBUTE		0x0053
#define HRC_MCSCRIPT_XMLDATA_SWITCHCMD_MISSING_FALLTHROUGH_ATTRIBUTE	0x0054
#define HRC_MCSCRIPT_XMLDATA_SWITCHCMD_FALLTHROUGH_MUSTBE_TRUE_OR_FALSE 0x0055
#define HRC_MCSCRIPT_XMLDATA_SWITCHCMD_MISSING_OPERATION_ATTRIBUTE		0x0056
#define HRC_MCSCRIPT_XMLDATA_SWITCHCMD_MISSING_VAR_ATTRIBUTE			0x0057
#define HRC_MCSCRIPT_XMLDATA_SWITCHCMD_INVALID_OPERATION				0x0058
#define HRC_MCSCRIPT_XMLDATA_SWITCHCMD_VAR_NOT_DOLLAR_PREFIX			0x0059
#define HRC_MCSCRIPT_XMLDATA_SWITCHCMD_VAR_MAYNOT_USE_ARRAY				0x0060

#define HRC_MCSCRIPT_XMLDATA_ELSEIFCMD_MISSING_TEST_ATTRIBUTE			0x0061
#define HRC_MCSCRIPT_XMLDATA_ELSECMD_MUST_FOLLOW_IFORELSEIF				0x0062
#define HRC_MCSCRIPT_XMLDATA_ELSEIFCMD_MUST_FOLLOW_IFORELSEIF			0x0063

#define HRC_MCSCRIPT_XMLDATA_MAPCMD_MISSING_NAME_ATTRIBUTE				0x0064
#define HRC_MCSCRIPT_XMLDATA_PAIRCMD_MISSING_KEY_ATTRIBUTE				0x0065
#define HRC_MCSCRIPT_XMLDATA_PAIRCMD_MISSING_VALUE_ATTRIBUTE			0x0066

#define HRC_MCSCRIPT_STATEMENT_INCORRECTLY_FORMED						0x2000
#define HRC_MCSCRIPT_STATEMENT_MULTIPARAM_NOTALLOWED					0x2001
#define HRC_MCSCRIPT_STATEMENT_WASNOT_TRANSLATED						0x2002
#define HRC_MCSCRIPT_STATEMENT_DOESNOT_BEGIN_WITH_DOLLAR				0x2003

#define HRC_MCSCRIPT_PROCNAME_NOT_FOUND									0x3000



class cNameSpaceID
{
public:
	const char *operator()();
private:
	std::string m_string;
};

class cMCScript;
class cMCScriptKeywordHandler
{
public:
	virtual int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer) = 0;
};

class cMCStatementParseElement
{
public:
	typedef enum SYMBOL_TYPE
	{ 
		ST_LEFT_PAREM,	// (
		ST_RIGHT_PAREM, // )
		ST_LEFT_BRACE,	// [
		ST_RIGHT_BRACE	// ]
	} eSymbolType;
	typedef enum STATEMENT_TYPE
	{
		ST_ARRAY,
		ST_MAP
	} eStatementType;
	eSymbolType m_symbol;
	eStatementType m_statementType;
	std::string m_string;
	int m_start_pos;
	int m_end_pos;
};

class cMCScript
{
public:
	cMCScript();
	~cMCScript();
	int LoadXML(const char *xmlFileName, bool bDisplayXMLParse);
	int LoadXML(std::string &contents, bool bDisplayXMLParse);
	int Translate(bool bDisplayTranslation, bool bTranslateOnly=false);
	const char *GetInvalidTagMessages() { return m_InvalidTagMessages.c_str(); }
	const char *GetTranslatedScript() { return m_TCLScript.c_str(); }
	const char *GetLastCommand() { return m_LastCommand.c_str(); }

	int TranslateStatement(std::string &sStatement);
	const char *GetVariableDeclarations() { return m_VariableDeclarations.c_str(); }
	void AddProcedure(std::string &sProcedureName);
	bool ExistsProcedure(std::string &sProcedureName);

	void IncreaseIndentDepth();
	void DecreaseIndentDepth();
	std::string GetIndentTabs();

	bool GetVarValue(std::string &sExpression, std::string &sVar, std::string &sValue);
	bool GetQoutedValue(std::string &sExpression, std::string &sValue);

	void SetMapMarker(int pos);
	int GetMapMarker() { return m_LastMarkMarker; }

private:
	bool m_bInitFailed;
	cXMLLiteParser m_xml;
	std::string m_TCLScript;
	std::string m_InvalidTagMessages;
	std::string m_LastCommand;
	std::map<std::string, cMCScriptKeywordHandler*> m_KeywordMap;
	std::map<std::string, bool> m_ProcedureMap;

	static const char *m_LegalNames[];
	std::stack<cMCStatementParseElement> m_StatementStack;
	std::queue<cMCStatementParseElement> m_StatementQueue;
	std::string m_VariableDeclarations;
	int m_iVariableCount;
	std::string m_IndentTabs;

	int m_LastMarkMarker;

	int PrepareKeywordMap();
	int ExecuteScriptValidationTests();
	int ValidateRoot();
	int ValidateLowercaseTags();
	int ValidateTagNames();
	int ValidateOpenCloseTags();

	int HandleRecursiveInclude(std::string &s);

	std::string CreateNewVariable();

	void GetClosestNameMatch(std::string &sName);

	int ParseNestedStatements(std::string &sStatement);
	std::string ProcessNestedStatements();
	int ProcessStatementFragment(std::string &subexpr, std::string::size_type index, 
								 std::string &statement, bool bIsArray);
	int GetProcedureName(std::string &sStatement, std::string &sProcName);

	void HandleArray(cMCStatementParseElement &leftElm, cMCStatementParseElement &rightElm, std::string &sStatement, std::string &sLastStatement);
	void HandleMap(cMCStatementParseElement &leftElm, cMCStatementParseElement &rightElm, std::string &sStatement, std::string &sLastStatement);
};

#endif //_MCSCRIPT_H
