#include "Client/ClientNetwork/ClientNetwork.h";
#include "Utilities/Interfaces/ILog.h";
#include "string.h";

ClientNetwork::ClientNetwork()
{

}
ClientNetwork::~ClientNetwork()
{

}

void ClientNetwork::Start()
{
	ThreadDesc threadDesc = { };
	threadDesc.pFunc = ClientNetwork::run;
	threadDesc.pData = this;
	strncpy(threadDesc.mThreadName, "NetworkRun", sizeof(threadDesc.mThreadName));
	initThread(&threadDesc, &threadHandle);
}
void ClientNetwork::Stop()
{
	isRunning = false;
	joinThread(threadHandle);
}

void ClientNetwork::run(void* threadData)
{
	SteamDatagramErrMsg dgrmErr;
	if (!GameNetworkingSockets_Init(nullptr, dgrmErr))
	{
		printf("GameNetworkingSockets_Init failed: %s", dgrmErr);
		return;
	}

	ClientNetwork* network = (ClientNetwork*)threadData;

	network->mp_networkingSockets = SteamNetworkingSockets();

	SteamNetworkingIPAddr szAddr;
	szAddr.ParseString("127.0.0.1:43110");

	SteamNetworkingConfigValue_t opt;
	opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
	network->m_hConnection = network->mp_networkingSockets->ConnectByIPAddress(szAddr, 1, &opt);

	if (network->m_hConnection == k_HSteamNetConnection_Invalid)
	{
		printf("ConnectByIPAddress failed");
		return;
	}

	LOGF(LogLevel::eINFO, "NETWORK EES RUNNIN");

	network->isRunning = true;

	while (network->isRunning)
	{
		network->pollIncomingMessages();
		network->mp_networkingSockets->RunCallbacks();
		network->processMessageQueue();
		threadSleep(10);
	}
}

void ClientNetwork::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pCallback)
{

}

void ClientNetwork::pollIncomingMessages()
{
	bool hasMessages = true;
	while (isRunning && hasMessages)
	{
		ISteamNetworkingMessage* msg = nullptr;
		int numMessages = mp_networkingSockets->ReceiveMessagesOnConnection(m_hConnection, &msg, 1);
		if (numMessages == 0)
		{
			hasMessages = false;
			break;
		}
		if (numMessages < 0)
		{
			LOGF(LogLevel::eERROR, "ReceiveMessagesOnConnection failed");
			Stop();
			break;
		}

		std::string str((const char*)msg->m_pData, msg->m_cbSize);
		GameMessage gmsg;
		gmsg.ParseFromString(str);
		msg->Release();

		switch (gmsg.msg_case())
		{
		case GameMessage::kCharacterDiscovery:
		{
			CharacterDiscovery cd = gmsg.characterdiscovery();

			//mp_gameSystem->ProcessCharacterDiscover(clientID, cd.name());
			break;
		}

		case GameMessage::kCharacterMove:
		{
			CharacterMove cm = gmsg.charactermove();
			/*mp_gameSystem->ProcessCharacterMove(
				clientID,
				vec3{ cm.mx(), cm.my(), cm.mz() },
				0.0f,
				cm.move_type(),
				vec3{ cm.x(), cm.y(), cm.z() },
				cm.loaction_area()
			);
			gmsg.clear_charactermove();*/
			break;
		}

		default:
			break;
		}
	}
}

void ClientNetwork::runCallbacks()
{

}

void ClientNetwork::processMessageQueue()
{

}
