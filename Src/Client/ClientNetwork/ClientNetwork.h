#ifndef ROI_GAME_CLIENT_NETWORK_H
#define ROI_GAME_CLIENT_NETWORK_H

#include "MessageTypes.pb.h";
#include <google/protobuf/io/coded_stream.h>;
#include "Game/ThirdParty/OpenSource/flecs/flecs.h";
#include "steam/steamnetworkingsockets.h";
#include "Utilities/Interfaces/IThread.h";

class ClientNetwork
{
private:
	bool isRunning = false;
	ThreadHandle threadHandle = nullptr;
	ISteamNetworkingSockets* mp_networkingSockets;
	HSteamNetConnection m_hConnection;

	static void run(void* threadData);
	static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pCallback);

	void pollIncomingMessages();
	void runCallbacks();
	void processMessageQueue();

public:
	ClientNetwork();
	~ClientNetwork();

	void Start();
	void Stop();
}

#endif //ROI_GAME_CLIENT_NETWORK_H