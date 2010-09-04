/* msgCourier.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

msgCourier.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

msgCourier.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with msgCourier.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file msgCourier.cpp
 @brief Built in handler for MsgCourier messages
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
 Built in handler for MsgCourier messages
*/

#include "core.h"
#include "exception.h"
#include "log.h"
#include "msgCourier.h"
#include <string>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>

#include "routingtag.h"
#include "xmlliteparser.h"

using namespace std;
using namespace boost;

#define COMMAND_NAME "MC"

char *cMsgCourier::GetCommandName()
{
	return COMMAND_NAME;
}

void cMsgCourier::OnStartup(cIMsgQueue *pMsgQueue, cISysInfo *pSysInfo, cILogger *pLogger)
{
	//cRoutingTag rt;
	//rt.SetTag("6afb4db2-3a37-41ce-9496-22f98a7d6dca");

	m_pMsgQueue = pMsgQueue;
	m_pSysInfo = pSysInfo;
}

void cMsgCourier::OnShutdown()
{
}

int cMsgCourier::OnProcess(cIMsg *pMessage)
{
	MARK_TRAVEL(pMessage);

	string::size_type i;
	string command;


	try
	{
		command = pMessage->GetCommand();

		//////////////////////////////
		// TEST IF ROUTABLE MESSAGE //
		//////////////////////////////
		cIMsg *pRoutedMsg = 0;
		if (((cMsg*)pMessage)->GetRemoteHandler() != 0)
		{
			LOG("Routing %s to remote handler", command.c_str());

			int RemoteSocketHandle = (((cMsg*)pMessage)->GetRemoteHandler())->m_pNode->m_TCPSocketHandle;
			cIMsg *pReply = m_pMsgQueue->CreateMessage();
			((cMsg*)pMessage)->DeepCopy((cMsg*)pReply);
			((cMsg*)pReply)->SetTCPSocketHandle(RemoteSocketHandle);

			//stringstream ss (stringstream::in | stringstream::out);
			//ss	<< RemoteSocketHandle;
			//((cMsg*)pReply)->SetSID(ss.str().c_str());

			((cMsg*)pReply)->SetConnectionType(cIMsg::MSG_CT_OB_TCP);
			pReply->DispatchMsg();
			pMessage->MarkProcessed();

			return HRC_COMMAND_HANDLER_OK;
		}

		//////////////////////////
		// TEST IF POST COMMAND //
		//////////////////////////
		i = command.find("POST");
		if (i != string::npos)
		{
			HandleXoolleMessage(pMessage);
			pMessage->MarkProcessed();
			return HRC_COMMAND_HANDLER_OK;
		}

		////////////////////////////////
		// TEST IF FORWARDING MESSAGE //
		////////////////////////////////
		i = command.find(".FORWARD");
		if (i != string::npos)
		{
			string s = pMessage->GetTo();
			int socketfd = cCore::GetInstance()->GetPeerRoutingMap()->GetSocket(s.c_str());
			cIMsg *pForward = m_pMsgQueue->CreateMessage();
			((cMsg*)pMessage)->DeepCopy((cMsg*)pForward);
			((cMsg*)pForward)->SetFrom(pMessage->GetFrom());
			((cMsg*)pForward)->SetTCPSocketHandle(socketfd);
			((cMsg*)pForward)->SetConnectionType(cIMsg::MSG_CT_OB_TCP);
			((cMsg*)pForward)->SetFormat(cIMsg::MSG_FORMAT_REQ);
			pForward->DispatchMsg();
			pMessage->MarkProcessed();
			return HRC_COMMAND_HANDLER_OK;
		}

		/////////////////////////////
		// TEST FOR REPLY MESSAGES //
		/////////////////////////////
		i = command.find("MCP/");
		if (i != string::npos)
		{
			int socketfd = cCore::GetInstance()->GetPeerRoutingMap()->GetSocket(pMessage->GetFrom());

			cIMsg *pReply = m_pMsgQueue->CreateMessage();
			((cMsg*)pMessage)->DeepCopy((cMsg*)pReply);
			((cMsg*)pReply)->SetTCPSocketHandle(socketfd);
			((cMsg*)pReply)->SetConnectionType(cIMsg::MSG_CT_OB_TCP);
			((cMsg*)pReply)->SetFormat(cIMsg::MSG_FORMAT_RES);
			pReply->DispatchMsg();
			pMessage->MarkProcessed();
			return HRC_COMMAND_HANDLER_OK;
		}

		////////////////////
		// TEST FOR ERROR //
		////////////////////
		i = command.find(".ERROR");
		if (i != string::npos)
		{
			cIMsg *pReply = m_pMsgQueue->CreateMessageReply(pMessage);
			pReply->SetResponseCode(500);
			((cMsg*)pReply)->SetKeepAlive(false);
			pReply->DispatchMsg();
			pMessage->MarkProcessed();
			return HRC_COMMAND_HANDLER_OK;
		}

		///////////////////
		// TEST FOR PING //
		///////////////////
		i = command.find(".PING");
		if (i != string::npos)
		{
			//LOG("Recieved PING from %s", pMessage->GetSourceIP());
			cIMsg *pReply = m_pMsgQueue->CreateMessageReply(pMessage);
			SET_TOUCH_CODE(pReply, 8);
			pReply->DispatchMsg();
		}

		///////////////////////
		// TEST FOR PRESENCE //
		///////////////////////
		i = command.find(".PRESENCE");
		if (i != string::npos)
		{
			/*
			cIMsg *pReply = m_pMsgQueue->CreateMessageReply(pMessage);
			pReply->SetResponseCode(200);
			pReply->DispatchMsg();
			*/

			cIMsg::MSG_REPLY_ACTION reply = pMessage->GetReplyAction();
			string::size_type j;

			j = command.find(".BROADCAST");
			if (j != string::npos)
			{
				bool bRegistered = false;

				string sInterface1 = cCore::GetInstance()->GetServerPrimaryAddr();
				string sInterface2 = cCore::GetInstance()->GetServerSecondaryAddr();

				string sFrom = pMessage->GetFrom();
				bool bRemote = true;
				if (sFrom.find(sInterface1) != string::npos)
					bRemote = false;
				else if (sFrom.find(sInterface2) != string::npos)
					bRemote = false;

				if (bRemote == true)
				{
					string::size_type idx = sFrom.find("@");
					string sMachineName = sFrom.substr(0, idx);

					string sMacAddress = pMessage->GetHeaderFieldValue("HWAddr");
					string sServices = pMessage->GetHeaderFieldValue("Services");

					cPresence *pPresence = cCore::GetInstance()->GetPresence();
					string sNodeKey = pPresence->UpdateNode(pMessage->GetSourceIP(), sMachineName.c_str(), sMacAddress.c_str(), sServices.c_str()); //  ,ePresenceStatus status = PS_ACTIVE);

					cService *pService = cCore::GetInstance()->GetService();
					typedef tokenizer<char_separator<char> > tokenizer;
					char_separator<char> sep(" ");
					tokenizer tokens(sServices, sep);
					tokenizer::iterator tok_iter;
					for (tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
					{
						if ((*tok_iter) != string("MC"))
						{
							bRegistered = pService->RegisterRemoteService(sNodeKey.c_str(), (*tok_iter).c_str(),0,0,0, true, true, pMessage->GetTCPSocketHandle());
							if (bRegistered == false)
							{
								pService->UnregisterRemoteService(sNodeKey.c_str(), (*tok_iter).c_str());
								bRegistered = pService->RegisterRemoteService(sNodeKey.c_str(), (*tok_iter).c_str(),0,0,0, true, true, pMessage->GetTCPSocketHandle());
								if (bRegistered == false)
								{
									cIMsg *pReply = m_pMsgQueue->CreateMessageReply(pMessage);
									pReply->SetResponseCode(500);
									pReply->DispatchMsg();
								}
								break;
							}
						}
					}
				}
				if (bRegistered == true)
				{
					cIMsg *pReply = m_pMsgQueue->CreateMessageReply(pMessage);
					pReply->SetResponseCode(200);
					pReply->DispatchMsg();
				}
			}
		}

		////////////////////////////////
		// TEST FOR PEER LIST REQUEST //
		////////////////////////////////
		i = command.find(".PEER.LIST");
		if (i != string::npos)
		{
			string sList;
			cIMsg *pReply = m_pMsgQueue->CreateMessageReply(pMessage);
			cService *pService = cCore::GetInstance()->GetService();
			cServiceEntry *pEntry = pService->QueryServiceFirst(pMessage->GetSubject());
			do
			{
				if (pEntry != 0 && pEntry->m_pNode->m_status == PS_ACTIVE)
				{
					sList.append(pEntry->m_pNode->m_IPAddress.c_str());
					sList.append(":3400\n");
				}
			} while (pEntry = pService->QueryServiceNext());

			if (sList.length() >= 0)
			{
				pReply->SetContentType("text/s-expression");
				pReply->SetContentLength(sList.length());
				pReply->SetContentPayload(sList.c_str(), sList.length());
			}

			MARK_TRAVEL(pReply);
			pReply->SetResponseCode(200);
			pReply->DispatchMsg();
		}

		pMessage->MarkProcessed();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return HRC_COMMAND_HANDLER_OK;
}

const char *cMsgCourier::OnProcessInternal(const char *pMessage)
{
	cPresence *pPresence = cCore::GetInstance()->GetPresence();
	return pPresence->GetPresenceData();
}

void cMsgCourier::HandleXoolleMessage(cIMsg* pMessage)
{
	string sPayload;
	sPayload = "<?xml version=\"1.0\"?>";
	sPayload += "<msgcourier application=\"xoolle\" >"; //<sessionID id=\"6afb4db2-3a37-41ce-9496-22f98a7d6dca\" /></msgcourier>";

	const char *pContent = pMessage->GetContentPayload();
	const char *pszMessage = strstr(pContent, "<?xml");

	if (pszMessage != NULL)
	{
		char *pData = NULL;
		cXMLLiteElement *pElm; 
		cXMLLiteParser xml;
		xml.Parse(pszMessage);
		xml.MoveHead();
		do 
		{
			/*
			pElm = xml.GetElement();
			pData = const_cast<char*>(pElm->GetElementValue());
			if (pData == NULL)
				LOG("|%s|", pElm->GetElementName());
			else
				LOG("|%s|[%s]", pElm->GetElementName(), pData);
			if (pElm->hasAttributes())
			{
				pElm->MoveHead();
				do 
				{
					cXMLLiteAttributePair *pAttr = pElm->GetAttributePair();
					pData = const_cast<char*>(pAttr->GetAttribValue());
					LOG("   attrib: |%s|[%s]", pAttr->GetAttribName(), pData);
				} while (pElm->MoveNext() == HRC_XMLLITELIST_OK);
			}
			if (pElm->hasCDATA())
			{
				LOG("   CDATA: %s", pElm->GetElementCDATA());
			}
			*/

			pElm = xml.GetElement();
			pData = const_cast<char*>(pElm->GetElementName());
			if (strstr(pData, "getSession") != NULL)
			{
				string msg;
				msg += "<sessionID id=\"";
				msg += pMessage->GetMsgID();
				msg += "\" />";
				sPayload += msg;
				LOG("Received: getSession command. Sending: %s", msg.c_str());
			}
			else if (strstr(pData, "userActivity") != NULL)
			{
				if (pElm->hasAttributes())
				{
					pElm->MoveHead();
					cXMLLiteAttributePair *pAttr = pElm->GetAttributePair();
					LOG("Received: userActivity command: [%s]", pAttr->GetAttribValue());
				}
			}
			else if (strstr(pData, "retrieveForm") != NULL)
			{
				if (pElm->hasAttributes())
				{
					string sFormData;
					string id;
					pElm->MoveHead();
					cXMLLiteAttributePair *pAttr = pElm->GetAttributePair();
					id = pAttr->GetAttribValue();

					if (strstr(id.c_str(), "2CF17DF3-0ED9-4fd9-9CC9-178823798959") != NULL)
					{
						sFormData =
							"<q>1) Are you thinking about updating more than one area of your home?\n"
							"<rb>Yes, start with one area at a time. Don't worry you will have a chance to design other areas of your home your next time thru Xoolle.\n"
							"<rb>No, okay continue on and have fun...\n"
							"<hp>For a qualified quote regional responses are required due to regional pricing therefore, your zip code must be supplied in order to gain a competent quote.";
					}
					else if (strstr(id.c_str(), "0E460765-9E71-49ce-9C44-5B76632499E4") != NULL)
					{
						sFormData =
							"<q>2) What are you envisioning for your home?\n"
							"<rb>Cosmetic Remodel (Cosmetically updating your home without Structural changes).\n"
							"<rb>Major Remodel (Cosmetically updating your home including Structural changes).\n"
							"<rb>Addition (Adding square footage to your home).\n"
							"<rb>New Construction (New structure on your property).\n"
							"<v>Please respond with a year represented in 4 digits.\n"
							"<hp>There are underling concerns related to construction that might impact your quote. For example, a 1972 home, and prior, might contain asbestos in the plaster, acoustic ceiling material, ducting, stucco and etc. installed at your home.";
					}
					else if (strstr(id.c_str(), "8E1D84AA-6DC5-414d-9D60-0AF9851E9CF5") != NULL)
					{
						sFormData =
							"<q>3) What type of home is yours? choose all that apply.\n"
							"<rb>Detached single family?\n"
							"<rb>Attached condo/townhome?\n"
							"<cb>Single level?\n"
							"<cb>Multi level?\n"
							"<v>Please respond\n"
							"<hp>The type of home your contemplating an alteration on requires some basic information that aid in the quote process.";
					}
					else if (strstr(id.c_str(), "7C257A08-6F4E-4099-B2DC-22F90AB8A2D6") != NULL)
					{
						sFormData =
							"<q>As requested lets go through the XoolleCalc again, simply choose a room or area of your home that needs work.\n"
							"<rb >Bathrooms? \n"
							"<rb>Kitchen? \n"
							"<rb>Living Family area?\n" 
							"<rb>Dining area? \n"
							"<rb>Master Suite?\n" 
							"<rb>Bedrooms? \n"
							"<rb>Game/Bonus room? \n"
							"<rb>Laundry/Mudroom area?\n" 
							"<rb>Attic/Basement?\n" 
							"<rb>Garage area\n" 
							"<rb>Exterior?\n"
							"<rb>Addition\n"
							"<v>Please respond.";
					}
					else if (strstr(id.c_str(), "80A318FE-3B2F-4eed-9157-4214F8834BE6") != NULL)
					{
						sFormData =
							"<q>5) This form will be modified?";
					}

					string msg;
					msg += "<formData><![CDATA[";
					msg += sFormData.c_str();
					msg += "]]></formData>";
					sPayload += msg;

					LOG("Received: retrieveForm command: [%s].  Sending: [%s]", pAttr->GetAttribValue(), msg.c_str());
				}
			} //retrieveForm


		} while (xml.MoveNext() == HRC_XMLLITELIST_OK);

		sPayload += "</msgcourier>";
		cIMsg *pReply = m_pMsgQueue->CreateMessage();
		((cMsg*)pMessage)->DeepCopy((cMsg*)pReply);
		((cMsg*)pReply)->SetTCPSocketHandle(pMessage->GetTCPSocketHandle());
		((cMsg*)pReply)->SetConnectionType(cIMsg::MSG_CT_OB_TCP);
		((cMsg*)pReply)->SetContentType("text/xml");
		((cMsg*)pReply)->SetContentPayload(sPayload.c_str(), sPayload.length());
		((cMsg*)pReply)->SetFormat(cIMsg::MSG_FORMAT_RES);
		((cMsg*)pReply)->SetKeepAlive(false);
		pReply->DispatchMsg();
	}
}


