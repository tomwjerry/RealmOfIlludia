#ifndef SERVER_NETWORK_H
#define SERVER_NETWORK_H

#include "Server/ServerGameSystem.h";

#include "MessageTypes.pb.h";

#include "steam/steamnetworkingsockets.h";
#include "Utilities/Math/MathTypes.h";

#include <map>;
#include <string>;
#include <thread>;
#include <vector>;

class ServerGameSystem;

class ServerNetwork
{
private:
	struct Client_t
	{
		std::string m_charName;
		std::string m_AccountUID;
		unsigned long long clientID;
	};

	ServerGameSystem* mp_gameSystem;

	std::map<HSteamNetConnection, Client_t> m_mapClients;
	std::map<unsigned long long, HSteamNetConnection> m_mapClientIds;
	std::vector<SteamNetworkingMessage_t *> m_messageQueue;
	std::thread m_thread;
	bool m_running = false;

	ISteamNetworkingSockets* mp_networkingSockets = nullptr;
	HSteamNetPollGroup m_hPollGroup = k_HSteamNetPollGroup_Invalid;

	void run();	

	void PollIncommingMessages();
	void HandleMessageQueue();
	void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

	void QueueMessage(std::vector<unsigned long long> receivers, int flags, const ::google::protobuf::Message* msg);

	static ServerNetwork* s_pCallbackInstance;
	static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);

public:
	ServerNetwork(ServerGameSystem* pGameSystem);
	~ServerNetwork();

	void Start();
	void Stop();

	void QueueCharacterDiscovery(std::vector<unsigned long long> receivers, unsigned long long characterId, std::string characterName);
	void QueueCharacterMovement(
		std::vector<unsigned long long> receivers,
		unsigned long long characterId,
		vec3 movement,
		float facing,
		int movetype,
		vec3 position,
		int loactionarea
	);
};

#endif
