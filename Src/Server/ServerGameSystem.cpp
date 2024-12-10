#include "Server/ServerGameSystem.h";

void ServerGameSystem::Start()
{
	// Then we shove in an dummy character
	m_characters.push_back("Dummy");
	m_charIdMap[1001] = 0;
}

void ServerGameSystem::Stop()
{
}

void ServerGameSystem::SetNetwork(ServerNetwork* pNetwork)
{
	mp_network = pNetwork;
}

void ServerGameSystem::ProcessCharacterDiscover(unsigned long long characterId, std::string characterName)
{
	std::map<unsigned long long, int>::iterator it = m_charIdMap.find(characterId);
	if (it == m_charIdMap.end())
	{
		m_charIdMap[characterId] = m_characters.size();
		m_clients.push_back(characterId);
	}

	m_characters.push_back(characterName);

	// Todo: Register character name

	for (int i = 0; i < m_characters.size(); i++)
	{
		std::map<unsigned long long, int>::iterator cit = m_charIdMap.find(characterId);

		if (cit != m_charIdMap.end())
		{
			mp_network->QueueCharacterDiscovery(m_clients, cit->second, m_characters[i]);
		}
	}
}

void ServerGameSystem::ProcessCharacterMove(
	unsigned long long characterId, vec3 movement, float facing, int movetype, vec3 position, int loactionarea)
{
	// TODO: Do stuff with movement


	// Likely something like this, get every region, for each region loop through all characters in that region and then their movements / positions

	mp_network->QueueCharacterMovement(m_clients, characterId, movement, facing, movetype, position, loactionarea);
	mp_network->QueueCharacterMovement(m_clients, 1001, vec3(0.5f, 0.0f, 0.5f), 0.0f, 1, vec3(0.5f, 0.0f, 0.5f), loactionarea);
}
