#ifndef SERVER_GAME_SYSTEM_H
#define SERVER_GAME_SYSTEM_H

#include <map>;
#include <string>;
#include <vector>;
#include "Utilities/Math/MathTypes.h";

#include "Server/ServerNetwork/ServerNetwork.h";

class ServerNetwork;

class ServerGameSystem
{
private:
	std::map<unsigned long long, int> m_charIdMap;
	std::vector<std::string> m_characters;
	std::vector<unsigned long long> m_clients;

	ServerNetwork* mp_network;

public:
	void Start();
	void Stop();

	void SetNetwork(ServerNetwork* pNetwork);

	void ProcessCharacterDiscover(unsigned long long characterId, std::string characterName);
	void ProcessCharacterMove(unsigned long long characterId, vec3 movement, float facing, int movetype, vec3 position, int loactionarea);
};

#endif
