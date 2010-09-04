/* mcscript.cpp
   Copyright (C) 2005 Carlos Justiniano

mcscript.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript.cpp
 @brief Message Courier Script
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script translates an XML script to a TCL implementation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>

#ifdef _PLATFORM_WIN32
	#define BOOST_NO_STD_LOCALE 1
#endif // _PLATFORM_WIN32

#include <boost/regex.hpp>

#include "log.h"
#include "exception.h"
#include "mcscript.h"
#include "mcscripthandlers.h"
#include "levenshtein.h"

using namespace std;
using namespace boost;

#ifdef _PLATFORM_LINUX
	#include "uuid.h"
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	#include <windows.h>
	#include <stdlib.h>
#endif //_PLATFORM_WIN32

const char *cMCScript::m_LegalNames[] =
{
	"break", "by", "case", "chars", "choose", "compare", "continue", "copy", "default",
	"elseif", "else", "end", "expr", "file", "foreach" , "for", "if", "import", "include", "incr", "key", 
	"language", "len", "list", "item", "map", "matchcase", "mcscript", "name", "operation", 
	"output", "param", "pair", "pattern", "procedure", "regex", "return", 
	"retvar", "test", "text", "trimleft", "trimright" "script", "start", 
	"statement", "string", "switch", "value", "variable", "var", "var1", "var2", 
	"while", "with",
	"*"
};

#define MCS_MAP(word, object)\
{\
	m_KeywordMap.insert(pair<string,cMCScriptKeywordHandler*>(string(word), \
						dynamic_cast<cMCScriptKeywordHandler*>(new object))); \
}

const char* cNameSpaceID::operator()()
{
	const char *pNamespace = NULL;
	
	try
	{	
	#ifdef _PLATFORM_LINUX
		#ifdef _DONT_USE_LINUX_UUID	
			char guid[40];
			sprintf(guid, "mcs_%X-0000-0000-0000-000000000000", time(0));
			m_string.assign(guid);
		#else
			uuid_t *out;
			char *pszUUID = NULL;
			uuid_create(&out);
			uuid_make(out, UUID_MAKE_V1);
			uuid_export(out, UUID_FMT_STR, (void**)&pszUUID, NULL);
			m_string.assign("msc_");
			m_string.assign(pszUUID);		
			uuid_destroy(out);
			free(pszUUID);	
		#endif //_DONT_USE_LINUX_UUID	
	#endif //_PLATFORM_LINUX

	#ifdef _PLATFORM_WIN32
		unsigned char *stringuuid;
		UUID Uuid;
		RPC_STATUS rt;
		rt = UuidCreate(&Uuid);
		rt = UuidToString(&Uuid, &stringuuid);
		m_string = "msc_";
		m_string += (const char*)stringuuid;
		RpcStringFree(&stringuuid);
	#endif //_PLATFORM_WIN32
		pNamespace = m_string.c_str();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return pNamespace;
}

cMCScript::cMCScript()
: m_bInitFailed(false)
, m_iVariableCount(0)
, m_LastMarkMarker(-1)
{
	m_IndentTabs = "";
	if (PrepareKeywordMap() != HRC_MCSCRIPT_OK)
		m_bInitFailed = true;
}	

cMCScript::~cMCScript()
{
	try
	{	
		map<string,cMCScriptKeywordHandler*>::iterator it;
		for (it = m_KeywordMap.begin(); it != m_KeywordMap.end(); it++)
		{
			delete (*it).second;
		}
		m_KeywordMap.erase(m_KeywordMap.begin(), m_KeywordMap.end());
		m_ProcedureMap.erase(m_ProcedureMap.begin(), m_ProcedureMap.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

int cMCScript::PrepareKeywordMap()
{
	int ret = HRC_MCSCRIPT_OK;
	try
	{
			MCS_MAP("by", cMCAttributeHandler());
		MCS_MAP("break", cMCBreakHandler());
		MCS_MAP("call", cMCCallHandler());
		MCS_MAP("case", cMCCaseHandler());
			MCS_MAP("chars", cMCAttributeHandler());
		MCS_MAP("continue", cMCContinueHandler());
			MCS_MAP("copy", cMCAttributeHandler());
		MCS_MAP("default", cMCDefaultHandler());
			MCS_MAP("end", cMCAttributeHandler());
		MCS_MAP("elseif", cMCElseIfHandler());
		MCS_MAP("else", cMCElseHandler());
		MCS_MAP("expr", cMCExprHandler());
			MCS_MAP("fallthrough", cMCAttributeHandler());
			MCS_MAP("file", cMCAttributeHandler());
		MCS_MAP("foreach", cMCForeachHandler());
		MCS_MAP("for", cMCForHandler());
		MCS_MAP("if", cMCIfHandler());
		MCS_MAP("import", cMCImportHandler());
		MCS_MAP("incr", cMCIncrHandler());
			MCS_MAP("incr", cMCAttributeHandler());
			MCS_MAP("key", cMCAttributeHandler());
			MCS_MAP("len", cMCAttributeHandler());
		MCS_MAP("list", cMCListHandler());
			MCS_MAP("list", cMCAttributeHandler());
			MCS_MAP("item", cMCAttributeHandler());
			MCS_MAP("language", cMCAttributeHandler());
			MCS_MAP("matchcase", cMCAttributeHandler());
		MCS_MAP("map", cMCMapHandler());
		MCS_MAP("mcscript", cMCMcscriptHandler());
			MCS_MAP("name", cMCAttributeHandler());
			MCS_MAP("operation", cMCAttributeHandler());
		MCS_MAP("output", cMCOutputHandler());
		MCS_MAP("pair", cMCPairHandler());
			MCS_MAP("param", cMCAttributeHandler());
			MCS_MAP("pattern", cMCAttributeHandler());
		MCS_MAP("procedure", cMCProcedureHandler());
			MCS_MAP("proc-name", cMCAttributeHandler());
		MCS_MAP("regex", cMCRegexHandler());
		MCS_MAP("return", cMCReturnHandler());
			MCS_MAP("retvar", cMCAttributeHandler());
		MCS_MAP("script", cMCScriptHandler());
			MCS_MAP("start", cMCAttributeHandler());
			MCS_MAP("statement", cMCAttributeHandler());
		MCS_MAP("string", cMCStringHandler());
		MCS_MAP("switch", cMCSwitchHandler());
			MCS_MAP("test", cMCAttributeHandler());
			MCS_MAP("text", cMCAttributeHandler());
			MCS_MAP("trimleft", cMCAttributeHandler());
			MCS_MAP("trimright", cMCAttributeHandler());
			MCS_MAP("value", cMCAttributeHandler());
		MCS_MAP("variable", cMCVariableHandler());
			MCS_MAP("var", cMCAttributeHandler());
			MCS_MAP("var1", cMCAttributeHandler());
			MCS_MAP("var2", cMCAttributeHandler());
		MCS_MAP("while", cMCWhileHandler());
			MCS_MAP("with", cMCAttributeHandler());
	}
	catch (...)
	{
		LOGALL("HRC_MSCRIPT_MEMORY_ERROR");	
		ret = HRC_MCSCRIPT_MEMORY_ERROR;
	}
	return ret;
}

int cMCScript::LoadXML(const char *xmlFileName, bool bDisplayXMLParse)
{
	if (m_bInitFailed)
		return HRC_MCSCRIPT_INIT_FAILED;

	int ret = HRC_MCSCRIPT_OK;
	string sXML;
	char buf[2048];
	int iread;
	FILE *fp = fopen(xmlFileName,"rt");
	if (fp)
	{
		while ((iread = fread(buf,1,2048,fp)))
		{
			sXML.append(buf, iread);
		}
		fclose(fp);
		ret = LoadXML(sXML, bDisplayXMLParse);
	}
	else
	{
		ret = HRC_MCSCRIPT_FILE_ERROR;
	}
	return ret;
}

int cMCScript::LoadXML(std::string &contents, bool bDisplayXMLParse)
{
	if (m_bInitFailed)
		return HRC_MCSCRIPT_INIT_FAILED;

	int ret = HRC_MCSCRIPT_OK;
	try
	{
		m_ProcedureMap.erase(m_ProcedureMap.begin(), m_ProcedureMap.end());

		// handle recursive >include>
		HandleRecursiveInclude(contents);

		// remove [CR]'s 
		string::size_type pos;
		while ((pos = contents.find(0x0d)) != string::npos)
			contents.erase(pos,1);
		// remove [TAB]'s 
		while ((pos = contents.find(0x09)) != string::npos)
			contents.erase(pos,1);

		ret = m_xml.Parse(contents.c_str());
		switch (ret)
		{
			case HRC_XMLLITEPARSER_OK:
				ret = HRC_MCSCRIPT_OK;
				break;
			case HRC_XMLLITEPARSER_ALLOC:
				ret = HRC_MCSCRIPT_MEMORY_ERROR;
				break;
		};

		if (ret != HRC_XMLLITEPARSER_OK)
			return ret;

		if (bDisplayXMLParse)
		{
			printf("\nXML PARSE:\n");
			char *pData = 0;
			cXMLLiteElement *pElm; 
			m_xml.MoveHead();
			do 
			{
				pElm = m_xml.GetElement();
				pData = const_cast<char*>(pElm->GetElementValue());
				if (pData == NULL)
					printf("|%s|\n", pElm->GetElementName());
				else
					printf("|%s|[%s]\n", pElm->GetElementName(), pData);
				if (pElm->hasAttributes())
				{
					pElm->MoveHead();
					do 
					{
						cXMLLiteAttributePair *pAttr = pElm->GetAttributePair();
						pData = const_cast<char*>(pAttr->GetAttribValue());
						if (pData == NULL || strlen(pData) == 0)
							printf("   attrib: |%s|[]\n", pAttr->GetAttribName());
						else
							printf("   attrib: |%s|[%s]\n", pAttr->GetAttribName(), pData);
					} while (pElm->MoveNext() == HRC_XMLLITELIST_OK);
				}
				if (pElm->hasCDATA())
				{
					printf("   CDATA: %s\n", pElm->GetElementCDATA());
				}
			} while (m_xml.MoveNext() == HRC_XMLLITELIST_OK);
			printf("\n");
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return ret;
}


int cMCScript::HandleRecursiveInclude(string &s)
{
	int ret = 0;
	char buf[2048];
	int iread;
	string::size_type mark1 = 0;
	string::size_type mark2 = 0;
	string sExpression;
	string sValue;
	string sInsert;

	sInsert = "";

	while ((mark1 = s.find("<include")) != string::npos)
	{
		mark2 = s.find("/>", mark1);
		if (mark2 != string::npos)
		{
			mark2 += 4;

			sExpression = s.substr(mark1, mark2);
			if (GetQoutedValue(sExpression, sValue))
			{
				sInsert = "";
				FILE *fp = fopen(sValue.c_str(),"rt");
				if (fp)
				{
					while ((iread = fread(buf,1,2048,fp)))
					{
						sInsert.append(buf, iread);
					}
					fclose(fp);
				}
			}

			s.erase(mark1, mark2 - mark1);
			if (sInsert.length() > 0)
			{
				if ((mark2 = sInsert.find("<mcscript>")) != string::npos)
					sInsert.erase(mark2, 10);
				if ((mark2 = sInsert.find("</mcscript>")) != string::npos)
					sInsert.erase(mark2, 11);
				sInsert += "\n";
				s.insert(mark1, sInsert);
			}
		}
	}
	return ret;
}

int cMCScript::Translate(bool bDisplayTranslation, bool bTranslateOnly)
{
	m_iVariableCount = 0;

	// validate scripts
	int ret = HRC_MCSCRIPT_OK;
	ret = ExecuteScriptValidationTests();
	if (ret != HRC_MCSCRIPT_OK)
		return ret;

	try
	{
		// begin translation
		m_TCLScript = "";
		char *pData = 0;
		cXMLLiteElement *pElm; 
		cMCScriptKeywordHandler *pHandler;
		map<string,cMCScriptKeywordHandler*>::iterator it;
		m_xml.MoveHead();
		do 
		{
			pElm = m_xml.GetElement();
			pData = const_cast<char*>(pElm->GetElementName());
			if (pData[0] == '/' && strcmp(pData, "/script")!=0)
			{
				if (pData[1] != '>')
				{
					m_LastCommand = pData+1;
					DecreaseIndentDepth();
					m_TCLScript += "\n";
					m_TCLScript += GetIndentTabs();
					m_TCLScript += "}";
				}
			}
			else
			{
				it = m_KeywordMap.find(pData);
				if (it != m_KeywordMap.end())
				{
					pHandler = (*it).second;
					ret = pHandler->Process(this, &m_xml, &m_TCLScript);
					if (ret != HRC_MCSCRIPT_OK)
						return ret;
				}
				m_LastCommand = pData;
			}
		} while (m_xml.MoveNext() == HRC_XMLLITELIST_OK);

		if (bDisplayTranslation)
		{
			if (!bTranslateOnly)
				printf("\nTRANSLATED CODE: \n");
			printf(m_TCLScript.c_str());
			printf("\n\n");
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return ret;
}

int cMCScript::ExecuteScriptValidationTests()
{
	int ret = HRC_MCSCRIPT_OK;
	m_xml.MoveHead();
	cXMLLiteElement *pElm = m_xml.GetElement();
	if (pElm == 0)
		return HRC_MCSCRIPT_XMLDATA_EMPTY;

	ret = ValidateRoot();
	if (ret != HRC_MCSCRIPT_OK)
		return ret;

	ret = ValidateOpenCloseTags();
	if (ret != HRC_MCSCRIPT_OK)
		return ret;

	ret = ValidateLowercaseTags();
	if (ret != HRC_MCSCRIPT_OK)
		return ret;

	ret = ValidateTagNames();
	if (ret != HRC_MCSCRIPT_OK)
		return ret;

	return ret;
}

int cMCScript::ValidateRoot()
{
	int ret = HRC_MCSCRIPT_OK;
	char *pData = 0;
	cXMLLiteElement *pElm; 
	m_xml.MoveHead();

	pElm = m_xml.GetElement();
	pData = const_cast<char*>(pElm->GetElementName());
	if (pData == 0)
	{
			ret = HRC_MCSCRIPT_XMLDATA_INVALIDROOT;
	}
	else if (strcmp(pData,"xml")==0)
	{
		m_xml.MoveNext();
		pElm = m_xml.GetElement();
		pData = const_cast<char*>(pElm->GetElementName());
		if (strcmp(pData,"mcscript")!=0)
			ret = HRC_MCSCRIPT_XMLDATA_INVALIDROOT;
	}
	else
	{
		if (strcmp(pData,"mcscript")!=0)
			ret = HRC_MCSCRIPT_XMLDATA_INVALIDROOT;
	}
	return ret;
}

int cMCScript::ValidateLowercaseTags()
{
	int ret = HRC_MCSCRIPT_OK;
	char *pData = 0;
	cXMLLiteElement *pElm; 
	m_xml.MoveHead();
	do 
	{
		pElm = m_xml.GetElement();
		pData = const_cast<char*>(pElm->GetElementName());
		if (pData[0] < 'a' && pData[0] != '/') // then tag is in uppercase
			return HRC_MCSCRIPT_XMLDATA_NEEDLOWERCASE_ELEMENT;
		if (pElm->hasAttributes())
		{
			pElm->MoveHead();
			do 
			{
				cXMLLiteAttributePair *pAttr = pElm->GetAttributePair();
				pData = const_cast<char*>(pAttr->GetAttribName());
				if (pData[0] < 'a' && pData[0] != '/') // then attribute is in uppercase
					return HRC_MCSCRIPT_XMLDATA_NEEDLOWERCASE_ATTRIBUTE;
			} while (pElm->MoveNext() == HRC_XMLLITELIST_OK);
		}
	} while (m_xml.MoveNext() == HRC_XMLLITELIST_OK);
	return ret;
}

int cMCScript::ValidateTagNames()
{
	int ret = HRC_MCSCRIPT_OK;
	char *pData = 0;
	string sName;
	cXMLLiteElement *pElm; 
	map<string,cMCScriptKeywordHandler*>::iterator it;

	try
	{
		m_InvalidTagMessages = "";
		m_xml.MoveHead();
		do 
		{
			pElm = m_xml.GetElement();
			pData = const_cast<char*>(pElm->GetElementName());
			if (strcmp(pData, "/>") != 0)
			{
				bool bTerm;
				if (pData[0] == '/')
				{
					bTerm = true;
					sName = pData+1;
				}
				else
				{
					bTerm = false;
					sName = pData;
				}
				it = m_KeywordMap.find(sName);
				if (it == m_KeywordMap.end())
				{
					m_InvalidTagMessages += "[-] Invalid element name '";
					if (bTerm)
						m_InvalidTagMessages += "/";
					m_InvalidTagMessages += sName;
					m_InvalidTagMessages += "' detected.";

					string levSearch = sName;
					GetClosestNameMatch(levSearch);
					if (levSearch.length() > 0)
					{
						m_InvalidTagMessages += " Did you mean '";
						if (bTerm)
							m_InvalidTagMessages += "/";
						m_InvalidTagMessages += levSearch;
						m_InvalidTagMessages += "'?\n";
					}
					else
					{
						m_InvalidTagMessages += "\n";
					}
				}
			}

			if (pElm->hasAttributes())
			{
				pElm->MoveHead();
				do 
				{
					cXMLLiteAttributePair *pAttr = pElm->GetAttributePair();
					pData = const_cast<char*>(pAttr->GetAttribName());
					sName = pData;
					it = m_KeywordMap.find(sName);
					if (it == m_KeywordMap.end())
					{
						m_InvalidTagMessages += "[-] Invalid attribute name '";
						m_InvalidTagMessages += sName;
						m_InvalidTagMessages += "' detected.";

						string levSearch = sName;
						GetClosestNameMatch(levSearch);
						if (levSearch.length() > 0)
						{
							m_InvalidTagMessages += " Did you mean '";
							m_InvalidTagMessages += levSearch;
							m_InvalidTagMessages += "'?\n";
						}
						else
						{
							m_InvalidTagMessages += "\n";
						}
					}
				} while (pElm->MoveNext() == HRC_XMLLITELIST_OK);
			}
		} while (m_xml.MoveNext() == HRC_XMLLITELIST_OK);

		if (m_InvalidTagMessages.length() != 0)
			ret = HRC_MCSCRIPT_XMLDATA_INVALIDTAGS;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return ret;
}

int cMCScript::ValidateOpenCloseTags()
{
	int ret = HRC_MCSCRIPT_OK;
	int openTagCount = 0;
	int closeTagCount = 0;
	char *pData = 0;
	cXMLLiteElement *pElm; 
	m_xml.MoveHead();
	do 
	{
		pElm = m_xml.GetElement();
		pData = const_cast<char*>(pElm->GetElementName());
		if (pData[0] != '/')
			openTagCount++;
		else
			closeTagCount++;
	} while (m_xml.MoveNext() == HRC_XMLLITELIST_OK);

	if (openTagCount != closeTagCount)
		ret = HRC_MCSCRIPT_XMLDATA_OPENCLOSE_TAGMISMATCH;
	return ret;
}

void cMCScript::GetClosestNameMatch(string &sName)
{
	int i = 0;
	int idx = -1;
	int closest = 999999;
	int distance;
	string sValidName;
	int misthreshhold = 2;

	try
	{
		while (m_LegalNames[i][0] != '*')
		{
			sValidName = m_LegalNames[i];
			cLevenshtein lev;
			distance = lev.distance(sName, sValidName);
			if (distance < closest)
			{
				closest = distance;
				idx = i;
			}
			i++;
		}

		if (idx != -1 && closest < misthreshhold )
			sName = m_LegalNames[idx];
		else
			sName = "";
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

int cMCScript::TranslateStatement(string &sStatement)
{
	int ret;
	bool bTranslated = false;
	match_results<std::string::const_iterator> what;
	string subexpr;
	string::size_type index;

	try
	{
//		if (sStatement[0] != '$')
//			return HRC_MCSCRIPT_STATEMENT_DOESNOT_BEGIN_WITH_DOLLAR;

		m_VariableDeclarations = "";

		regex funcPattern("\\$[\\w]*\\([^)]*[\\)]+");
		while ((regex_search(sStatement, what, funcPattern, match_default)))
		{
			if (what[0].matched)
			{
				subexpr = what[0];
				index = what[0].first - sStatement.begin();
				ProcessStatementFragment(subexpr, index, sStatement, false);
				bTranslated = true;
			}
		}

		regex arrayPattern("\\$[\\w]*\\[[^]]*[\\]]+");
		while ((regex_search(sStatement, what, arrayPattern, match_default)))
		{
			if (what[0].matched)
			{
				subexpr = what[0];
				index = what[0].first - sStatement.begin();
				ProcessStatementFragment(subexpr, index, sStatement, true);
				bTranslated = true;
			}
		}

		regex mapPattern("\\#[\\w]*\\[[^]]*[\\]]+");
		while ((regex_search(sStatement, what, mapPattern, match_default)))
		{
			if (what[0].matched)
			{
				subexpr = what[0];
				index = what[0].first - sStatement.begin();
				ProcessStatementFragment(subexpr, index, sStatement, true);
				bTranslated = true;
			}
		}

		if (!bTranslated)
			ret = HRC_MCSCRIPT_STATEMENT_WASNOT_TRANSLATED;
		else
			ret = HRC_MCSCRIPT_OK;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return ret;
}

int cMCScript::ProcessStatementFragment(string &subexpr, string::size_type index, string &statement, bool bIsArray)
{
	int ret;
	string sVariable;
	string sTemp;

	try
	{
		statement.erase(index, subexpr.length());

		sVariable = CreateNewVariable();
		statement.insert(index, sVariable);

		ret = ParseNestedStatements(subexpr);
		if (ret != HRC_MCSCRIPT_OK)
			return ret;

		sTemp = ProcessNestedStatements();
		if (bIsArray)
		{
			m_VariableDeclarations.append("\n");
			m_VariableDeclarations.append(GetIndentTabs());
			m_VariableDeclarations += "set ";
			m_VariableDeclarations.append(sVariable, 1, sVariable.length());
			m_VariableDeclarations += " ";
			m_VariableDeclarations += sTemp;
			m_VariableDeclarations += "\n";
		}
		else
		{
			m_VariableDeclarations.append("\n");
			m_VariableDeclarations.append(GetIndentTabs());
			m_VariableDeclarations += "set ";
			m_VariableDeclarations.append(sVariable, 1, sVariable.length());
			m_VariableDeclarations += " ";
			m_VariableDeclarations += sTemp;
			m_VariableDeclarations += "\n";
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}

	return HRC_MCSCRIPT_OK;
}

int cMCScript::ParseNestedStatements(string &sStatement)
{
	char ch;
	string text;
	string::size_type pos = 0;
	string::size_type mark = pos;

	try
	{
		if ((sStatement.find_first_of(",", 0) != string::npos) &&
			(sStatement.find_first_of("(,", 0) != string::npos) &&
			(sStatement.find_first_of("'\"", 0) == string::npos))
			return HRC_MCSCRIPT_STATEMENT_MULTIPARAM_NOTALLOWED;

		while (!m_StatementStack.empty())
			m_StatementStack.pop();
		while (!m_StatementQueue.empty())
			m_StatementQueue.pop();
	
		while ((pos = sStatement.find_first_of("()[]", mark)) != string::npos)
		{
			ch = sStatement[pos];
			text = sStatement.substr(mark, pos-mark);

			cMCStatementParseElement elm;
			if (text[0] == '$')
				elm.m_statementType = cMCStatementParseElement::ST_ARRAY;
			else if (text[0] == '#')
				elm.m_statementType = cMCStatementParseElement::ST_MAP;
			elm.m_start_pos = mark;
			elm.m_end_pos = pos-mark;
			elm.m_string = text;
			if (ch == '(')
			{
				elm.m_symbol = cMCStatementParseElement::ST_LEFT_PAREM;
				if (elm.m_string[0] == '$')
					elm.m_string.erase(0,1);
				m_StatementStack.push(elm);
			}
			else  if (ch == '[')
			{
				elm.m_symbol = cMCStatementParseElement::ST_LEFT_BRACE;
				m_StatementStack.push(elm);
			}
			else if (ch == ')')
			{
				elm.m_symbol = cMCStatementParseElement::ST_RIGHT_PAREM;
				m_StatementQueue.push(elm);
			}
			else if (ch == ']')
			{
				elm.m_symbol = cMCStatementParseElement::ST_RIGHT_BRACE;
				m_StatementQueue.push(elm);
			}
			mark = pos + 1;
		}

		if (m_StatementStack.size() != m_StatementQueue.size())
			return HRC_MCSCRIPT_STATEMENT_INCORRECTLY_FORMED;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return HRC_MCSCRIPT_OK;
}

string cMCScript::ProcessNestedStatements()
{
	string sStatement = "";
	try
	{	
		if (m_StatementStack.size() != m_StatementQueue.size())
			return "";
		if (m_StatementStack.size() == 0 || m_StatementQueue.size() == 0)
			return "";

		cMCStatementParseElement leftElm;
		cMCStatementParseElement rightElm;
		string sLastStatement = "";
		while (m_StatementStack.size() != 0)
		{
			leftElm = m_StatementStack.top();
			rightElm = m_StatementQueue.front();

			if (leftElm.m_statementType == cMCStatementParseElement::ST_ARRAY)
				HandleArray(leftElm, rightElm, sStatement, sLastStatement);
			else if (leftElm.m_statementType == cMCStatementParseElement::ST_MAP)
				HandleMap(leftElm, rightElm, sStatement, sLastStatement);

			m_StatementStack.pop();
			m_StatementQueue.pop();
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return sStatement;
}

void cMCScript::HandleArray(cMCStatementParseElement &leftElm, cMCStatementParseElement &rightElm, string &sStatement, string &sLastStatement)
{
	if (rightElm.m_symbol == cMCStatementParseElement::ST_RIGHT_BRACE)
	{
		if (rightElm.m_string.length() == 0)
		{
			sStatement = "[lindex ";
			sStatement += leftElm.m_string;
			sStatement += " ";
			sStatement += sLastStatement;
			sStatement += "]";
		}
		else
		{
			sStatement = "[lindex ";
			sStatement += leftElm.m_string;
			sStatement += " ";
			sStatement += rightElm.m_string;
			sStatement += "]";
		}
		sLastStatement = sStatement;
	}
	else if (rightElm.m_symbol == cMCStatementParseElement::ST_RIGHT_PAREM)
	{
		if (rightElm.m_string.length() == 0 ||
			rightElm.m_string.find_first_of(",", 0) != string::npos)
		{
			sStatement = "[";
			sStatement += leftElm.m_string;
			sStatement += " ";
			sStatement += sLastStatement;
			sStatement += "]";
		}
		else
		{
			sStatement = "[";
			sStatement += leftElm.m_string;
			sStatement += " ";
			sStatement += rightElm.m_string;
			sStatement += "]";
		}
		sLastStatement = sStatement;			
	}
}

void cMCScript::HandleMap(cMCStatementParseElement &leftElm, cMCStatementParseElement &rightElm, string &sStatement, string &sLastStatement)
{
	if (rightElm.m_symbol == cMCStatementParseElement::ST_RIGHT_BRACE)
	{
		if (rightElm.m_string.length() == 0)
		{
			sStatement = "$";
			sStatement += leftElm.m_string.substr(1, leftElm.m_string.length() - 1);
			sStatement += "(";
			sStatement += sLastStatement;
			sStatement += ")";
		}
		else
		{
			sStatement = "$";
			sStatement += leftElm.m_string.substr(1, leftElm.m_string.length() - 1);
			sStatement += "(";
			sStatement += rightElm.m_string;
			sStatement += ")";
		}
		sLastStatement = sStatement;
	}
	else if (rightElm.m_symbol == cMCStatementParseElement::ST_RIGHT_PAREM)
	{
		if (rightElm.m_string.length() == 0 ||
			rightElm.m_string.find_first_of(",", 0) != string::npos)
		{
			sStatement = "$";
			sStatement += leftElm.m_string.substr(1, leftElm.m_string.length() - 1);
			sStatement += "(";
			sStatement += sLastStatement;
			sStatement += ")";
		}
		else
		{
			sStatement = "$";
			sStatement += leftElm.m_string;
			sStatement += "(";
			sStatement += rightElm.m_string;
			sStatement += ")";
		}
		sLastStatement = sStatement;			
	}
}

string cMCScript::CreateNewVariable()
{
	stringstream var(stringstream::out);
	try
	{
		var << "$__variable" << ++m_iVariableCount;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return var.str();
}

void cMCScript::AddProcedure(string &sProcedureName)
{
	try
	{	
		m_ProcedureMap.insert(pair<string,bool>(sProcedureName, true));
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

bool cMCScript::ExistsProcedure(string &sProcedureName)
{
	bool bRet;

	try
	{	
		string sProcName = "";
		int ret = GetProcedureName(sProcedureName, sProcName);
		if (ret != HRC_MCSCRIPT_OK)
			return false;
		map<string,bool>::iterator it;
		it = m_ProcedureMap.find(sProcName);
		bRet = (it == m_ProcedureMap.end()) ? false : true;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return bRet;
}

int cMCScript::GetProcedureName(string &sStatement, string &sProcName)
{
	try
	{	
		match_results<std::string::const_iterator> what;
		string subexpr;

		regex funcPattern("(\\$[\\w]*)");

		if ((regex_search(sStatement, what, funcPattern, match_default)))
		{
			if (what[0].matched)
			{
				subexpr = what[0];
				sProcName = subexpr.c_str() + 1;
				return HRC_MCSCRIPT_OK;
			}
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return HRC_MCSCRIPT_PROCNAME_NOT_FOUND;
}


void cMCScript::IncreaseIndentDepth()
{
	m_IndentTabs += "\t";
}

void cMCScript::DecreaseIndentDepth()
{
	if (m_IndentTabs.length() > 0)
	{
		m_IndentTabs = m_IndentTabs.substr(0, m_IndentTabs.length() - 1);
	}
}

string cMCScript::GetIndentTabs()
{
	if (m_IndentTabs.length() == 0)
		return "";
	return m_IndentTabs;
}

bool cMCScript::GetVarValue(string &sExpression, string &sVar, string &sValue)
{
	bool bRet = true;
	try
	{	
		sVar = "";
		sValue = "";
		match_results<std::string::const_iterator> what;
		string subexpr;

		regex varPattern("[^\\$]([a-zA-Z0-9\\-\\_]*)");
		regex valuePattern("[\\$a-zA-Z0-9\\-\\_]+");
		string sSearch = sExpression;
		if ((regex_search(sSearch, what, varPattern, match_default)))
		{
			if (what[0].matched)
			{
				sVar = what[0];
				sSearch = sExpression.substr(sVar.length()+1);
				if ((regex_search(sSearch, what, valuePattern, match_default)))
				{
					if (what[0].matched)
					{
						sValue = what[0];
					}
				}
			}
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return bRet;
}

bool cMCScript::GetQoutedValue(string &sExpression, string &sValue)
{
	bool bRet = true;
	try
	{	
		sValue = "";
		match_results<std::string::const_iterator> what;

		regex Pattern("\\x22[\\(\\)a-zA-Z0-9\\$\\-\\_\\.\\:\\x5c]+\\x22");
		if ((regex_search(sExpression, what, Pattern, match_default)))
		{
			if (what[0].matched)
			{
				sValue = what[0];
				sValue.erase(0, 1);
				sValue.erase(sValue.length()-1, 1);
			}
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return bRet;
}

void cMCScript::SetMapMarker(int pos)
{
	m_LastMarkMarker = pos;
}


