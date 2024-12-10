#include "Server/ServerGameSystem.h";

void main()
{
	ServerGameSystem* gameSystem = new ServerGameSystem();
	ServerNetwork* serverNetwork = new ServerNetwork(gameSystem);
	gameSystem->SetNetwork(serverNetwork);
	gameSystem->Start();
	serverNetwork->Start();
	
	bool shouldStop = false;
	while (shouldStop)
	{
		char szLine[4000];
		if (!fgets(szLine, sizeof(szLine), stdin))
		{
			shouldStop = szLine[0] == 'q';
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	serverNetwork->Stop();
	gameSystem->Stop();
}
