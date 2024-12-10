#include "Server/ServerNetwork/ServerNetwork.h"
#include <chrono>
#include <steam/isteamnetworkingutils.h>

class ServerGameSystem;

ServerNetwork* ServerNetwork::s_pCallbackInstance = nullptr;

ServerNetwork::ServerNetwork(ServerGameSystem* pGameSystem)
{
	mp_gameSystem = pGameSystem;
}

ServerNetwork::~ServerNetwork()
{
	if (m_running)
	{
		Stop();
	}
}

void ServerNetwork::Start()
{
	m_thread = std::thread(&ServerNetwork::run, this);
}

void ServerNetwork::Stop()
{
	m_running = false;
	m_thread.join();
}

void ServerNetwork::run()
{
	//Init sockets...
	SteamDatagramErrMsg dgrmErr;
	if (!GameNetworkingSockets_Init(nullptr, dgrmErr))
	{
		printf("GameNetworkingSockets_Init failed: %s", dgrmErr);
		return;
	}

	mp_networkingSockets = SteamNetworkingSockets();

	SteamNetworkingIPAddr serverLocalAddr;
	serverLocalAddr.Clear();
	serverLocalAddr.m_port = 43110;

	SteamNetworkingConfigValue_t opt;
	opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);

	HSteamListenSocket hListenSocket = mp_networkingSockets->CreateListenSocketIP(serverLocalAddr, 1, &opt);
	if (hListenSocket == k_HSteamListenSocket_Invalid)
	{
		return;
	}

	m_hPollGroup = mp_networkingSockets->CreatePollGroup();
	if (m_hPollGroup == k_HSteamNetPollGroup_Invalid)
	{
		return;
	}

	m_running = true;

	s_pCallbackInstance = this;

	printf("Networking initialized\n");

	while (m_running)
	{
		mp_networkingSockets->RunCallbacks();
		PollIncommingMessages();
		HandleMessageQueue();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	GameMessage	gameMessage;
	ServerInstruction* serverInstruction = gameMessage.mutable_serverinstruction();
	serverInstruction->set_instruction(1);
	std::string* buf = new std::string();
	gameMessage.SerializeToString(buf);
	gameMessage.clear_serverinstruction();
	gameMessage.Clear();

	for (std::pair<HSteamNetConnection, Client_t> client: m_mapClients)
	{
		mp_networkingSockets->SendMessageToConnection(client.first, buf->c_str(), buf->length() + 1, k_nSteamNetworkingSend_Reliable, nullptr);
		mp_networkingSockets->CloseConnection(client.first, 0, "Server shutdown", true);
	}

	buf->clear();

	m_mapClients.clear();

	mp_networkingSockets->CloseListenSocket(hListenSocket);
	hListenSocket = k_HSteamListenSocket_Invalid;

	mp_networkingSockets->DestroyPollGroup(m_hPollGroup);
	m_hPollGroup = k_HSteamNetPollGroup_Invalid;
}

void ServerNetwork::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	s_pCallbackInstance->OnConnectionStatusChanged(pInfo);
}

void ServerNetwork::PollIncommingMessages()
{
	bool hasMoreMessages = true;
	while (m_running && hasMoreMessages)
	{
		ISteamNetworkingMessage *pIncomingMessage = nullptr;
		int numMessages = mp_networkingSockets->ReceiveMessagesOnPollGroup(m_hPollGroup, &pIncomingMessage, 1);
		hasMoreMessages = numMessages > 0;

		if (numMessages < 0)
		{
			// TODO: Error...
			Stop();
		}

		if (numMessages == 0)
		{
			return;
		}

		std::string msg;
		msg.assign((const char*)pIncomingMessage->m_pData, pIncomingMessage->m_cbSize);
		pIncomingMessage->Release();

		GameMessage gmsg;
		gmsg.ParseFromString(msg);

		std::map<HSteamNetConnection, Client_t>::iterator it = m_mapClients.find(pIncomingMessage->m_conn);

		// Not found!
		if (it == m_mapClients.end())
		{
			continue;
		}

		int clientID = it->second.clientID;

		switch (gmsg.msg_case())
		{
		case GameMessage::kCharacterDiscovery:
		{
			CharacterDiscovery cd = gmsg.characterdiscovery();
			mp_gameSystem->ProcessCharacterDiscover(clientID, cd.name());
			break;
		}

		case GameMessage::kCharacterMove:
		{
			CharacterMove cm = gmsg.charactermove();
			mp_gameSystem->ProcessCharacterMove(
				clientID,
				vec3{ cm.mx(), cm.my(), cm.mz() },
				0.0f,
				cm.move_type(),
				vec3{ cm.x(), cm.y(), cm.z() },
				cm.loaction_area()
			);
			gmsg.clear_charactermove();
			break;
		}

		default:
			break;
		}

		gmsg.Clear();
	}
}

void ServerNetwork::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	switch (pInfo->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_Connecting:
		// Check if we can accept the connection
		if (mp_networkingSockets->AcceptConnection(pInfo->m_hConn) != k_EResultOK)
		{
			mp_networkingSockets->CloseConnection(pInfo->m_hConn, 0, "Failed to accept connection", false);
			break;
		}

		// Asign to poll group
		if (!mp_networkingSockets->SetConnectionPollGroup(pInfo->m_hConn, m_hPollGroup))
		{
			mp_networkingSockets->CloseConnection(pInfo->m_hConn, 0, "Failed to give polly bicuits", false);
			break;
		}

		// Toss the dude in the map
		m_mapClientIds[m_mapClients.size()] = pInfo->m_hConn;
		m_mapClients[pInfo->m_hConn] = Client_t{ "", "", m_mapClients.size() };

		break;

	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		char* pszDebugLogAction;
		// Has we actually connected?
		if (pInfo->m_hConn == k_ESteamNetworkingConnectionState_Connected)
		{
			std::map<HSteamNetConnection, Client_t>::iterator it = m_mapClients.find(pInfo->m_hConn);

			if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
			{
				pszDebugLogAction = "Problem detected locally";
			}
			else
			{
				pszDebugLogAction = "Closed by peer";
			}

			if (it == m_mapClients.end())
			{
				strcat(pszDebugLogAction, " (not found)");
			}
			else
			{
				// Send a "Vannish" message
				GameMessage gameMessage;
				CharacterVannish* vannish = gameMessage.mutable_charactervannish();
				vannish->set_id(it->second.clientID);
				std::string* buf = new std::string();
				gameMessage.SerializeToString(buf);
				gameMessage.clear_charactervannish();
				gameMessage.Clear();

				for (std::pair<HSteamNetConnection, Client_t> client : m_mapClients)
				{
					mp_networkingSockets->SendMessageToConnection(client.first, buf->c_str(), buf->length() + 1, k_nSteamNetworkingSend_Reliable, nullptr);
				}
				buf->clear();

				m_mapClients.erase(it);
			}

			mp_networkingSockets->CloseConnection(pInfo->m_hConn, 0, pszDebugLogAction, false);
		}
		break;

	default:
		break;
	}
}

void ServerNetwork::HandleMessageQueue()
{
	SteamNetworkingMessage_t** msgs = new SteamNetworkingMessage_t*[m_messageQueue.size() + 1];
	for (int i = 0; i < m_messageQueue.size(); i++)
	{
		msgs[i] = m_messageQueue[i];
	}
	m_messageQueue.clear();
	mp_networkingSockets->SendMessages(m_messageQueue.size(), msgs, nullptr);
}

void ServerNetwork::QueueCharacterDiscovery(std::vector<unsigned long long> receivers, unsigned long long characterId, std::string characterName)
{
	GameMessage gameMessage;
	CharacterDiscovery* cd = gameMessage.mutable_characterdiscovery();
	cd->set_id(characterId);
	cd->set_name(characterName);

	QueueMessage(receivers, k_nSteamNetworkingSend_Reliable, &gameMessage);
	
	gameMessage.clear_characterdiscovery();
	gameMessage.Clear();
}

void ServerNetwork::QueueCharacterMovement(
	std::vector<unsigned long long> receivers,
	unsigned long long characterId,
	vec3 movement,
	float facing,
	int movetype,
	vec3 position,
	int loactionarea)
{
	GameMessage gameMessage;
	CharacterMove* cm = gameMessage.mutable_charactermove();
	cm->set_id(characterId);
	cm->set_mx(movement.getX());
	cm->set_my(movement.getY());
	cm->set_mz(movement.getZ());
	//cm->set_facing(facing);
	cm->set_move_type(movetype);
	cm->set_x(position.getX());
	cm->set_y(position.getY());
	cm->set_z(position.getZ());
	cm->set_loaction_area(loactionarea);

	QueueMessage(receivers, k_nSteamNetworkingSend_Unreliable, &gameMessage);

	gameMessage.clear_charactermove();
	gameMessage.Clear();
}

void ServerNetwork::QueueMessage(std::vector<unsigned long long> receivers, int flags, const ::google::protobuf::Message* msg)
{
	std::string* buf = new std::string();
	msg->SerializeToString(buf);

	for (unsigned long long& r : receivers)
	{
		std::map<unsigned long long, HSteamNetConnection>::iterator it = m_mapClientIds.find(r);
		if (it != m_mapClientIds.end())
		{
			SteamNetworkingMessage_t* msg = SteamNetworkingUtils()->AllocateMessage(buf->length() + 1);
			msg->m_conn = it->second;
			msg->m_nFlags = flags;
			msg->m_idxLane = 0;
			memcpy(msg->m_pData, buf->c_str(), buf->length() + 1);
			m_messageQueue.push_back(msg);
		}
	}

	buf->clear();
}
