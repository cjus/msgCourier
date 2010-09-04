/* sexprwrapper.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

sexprwrapper.cpp is a wrapper for SFSEXP: A Small, Fast S-Expression 
Library written by Matthew Sottile (matt@lanl.gov)

sexprwrapper.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

sexprwrapper.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with sexprwrapper.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file sexprwrapper.cpp
 @brief  S-Expression Wrapper
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include "sexprwrapper.h"

using namespace std;

cSExpr::cSExpr()
: m_sx(0)
{
}

cSExpr::~cSExpr()
{
	Reset();
}

void cSExpr::Reset()
{
	sexp_cleanup();
	destroy_sexp(m_sx);
	m_sx = 0;
}

int cSExpr::Parse(string &expression)
{
	int ret;
	if (m_sx)
		destroy_sexp(m_sx);
	m_sx = parse_sexp((char*)expression.c_str(), MAX_BUFFER_SIZE*2);
	if ((m_sx->ty == SEXP_VALUE && m_sx->val != 0) ||
		(m_sx->ty == SEXP_LIST && m_sx->list != 0))
	{
		ret = HRC_SEXPR_OK;
	}
	else
	{
		ret = HRC_PARSE_FAILED;
	}
	return ret;
}

const char *cSExpr::GetSExpr() 
{ 
	if (m_sx == 0)
		return 0;
	print_sexp(m_buffer, MAX_BUFFER_SIZE, m_sx);
	return m_buffer;
}

SEXPR_NODE cSExpr::BFSFind(string &atom, SEXPR_NODE node)
{
	return (SEXPR_NODE)bfs_find_sexp((char*)atom.c_str(), (node == 0) ? m_sx : (sexp_t*)node);
}

SEXPR_NODE cSExpr::DFSFind(string &atom, SEXPR_NODE node)
{
	return (SEXPR_NODE)find_sexp((char*)atom.c_str(), (node == 0) ? m_sx : (sexp_t*)node);
}

const char *cSExpr::GetAtomValue(SEXPR_NODE node)
{
	sexp_t *pNode = (node == 0) ? m_sx : (sexp_t*)node;
	if (pNode->ty != SEXP_VALUE)
		return (const char*)0;
	return (const char*)pNode->val;
}

SEXPR_NODE cSExpr::GetNextAtom(SEXPR_NODE node)
{
	if (node == 0) return 0;
	sexp_t *pNode = (sexp_t*)node;
	if (pNode->ty == SEXP_LIST)
	{
		pNode = pNode->list;
		return (SEXPR_NODE)pNode;
	}
	return (SEXPR_NODE)(pNode->next != 0 && pNode->next->ty == SEXP_VALUE) ? pNode->next : 0;
}

SEXPR_NODE cSExpr::GetNextList(SEXPR_NODE node)
{
	if (node == 0) return 0;
	sexp_t *pNode = (sexp_t*)node;
	return (SEXPR_NODE)(pNode->next != 0 && pNode->next->ty == SEXP_LIST) ? pNode->next : 0;
}

bool cSExpr::HasAtom(SEXPR_NODE node)
{
	if (node == 0) return false;
	sexp_t *pNode = (sexp_t*)node;
	return (pNode->next != 0 && pNode->next->ty == SEXP_VALUE) ? true : false;
}

bool cSExpr::HasList(SEXPR_NODE node)
{
	if (node == 0) return false;
	sexp_t *pNode = (sexp_t*)node;
	return (pNode->next != 0 && pNode->next->ty == SEXP_LIST) ? true : false;
}

SEXPR_NODE cSExpr::GetBaseNode()
{
	if (m_sx == 0) return 0;
	return (SEXPR_NODE)m_sx;
}

SEXPR_NODE cSExpr::GetNextNode(SEXPR_NODE node)
{
	if (node == 0) return false;
	sexp_t *pNode = (sexp_t*)node;
	return (pNode->next != 0) ? pNode->next : 0;
}

SEXPR_NODE cSExpr::AddList(SEXPR_NODE node)
{
	if (node == 0) return 0;
	sexp_t *pNode = (sexp_t*)node;

	while (pNode)
	{
		if (pNode->next == 0)
		{
			pNode->next = new_sexp_list(0);
			break;
		}
		pNode = pNode->next;
	}
	return (SEXPR_NODE)pNode->next;
}

SEXPR_NODE cSExpr::AddAtom(SEXPR_NODE node, string &atom)
{
	if (node == 0) return false;
	sexp_t *pNode = (sexp_t*)node;

	if (pNode->ty == SEXP_LIST && pNode->next == 0 && pNode->list == 0)
	{
		pNode->list = new_sexp_atom((char*)atom.c_str(), atom.length());
		return (SEXPR_NODE)pNode->list;
	}

	if (pNode->ty == SEXP_LIST && pNode->list != 0)
		pNode = pNode->list;
	else if (pNode->ty == SEXP_VALUE && pNode->next != 0)
		pNode = pNode->next;

	while (pNode)
	{
		if (pNode->next == 0)
		{
			pNode->next = new_sexp_atom((char*)atom.c_str(), atom.length());
			break;
		}
		pNode = pNode->next;
	}
	return (SEXPR_NODE)pNode->next;
}

