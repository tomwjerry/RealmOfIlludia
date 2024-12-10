#include "MessageTypes.h";

class ServerInstruction;

std::string MessageTypes::Serialize(MessageTypes mt)
{
	return "0";
};

void ServerInstruction::dummyfunc()
{
	MessageTypes* mt = new ServerInstruction();
}

MessageTypes* MessageTypes::Parse(std::string data)
{
	MessageTypes* mt = nullptr;
	ServerInstruction* si = nullptr;

	if (data.length() > 2)
	{
		MessageType messageTypeId = static_cast<MessageType>(stoi(data.substr(0, 2)));

		switch (messageTypeId)
		{
		case MessageType::MTServerInstruction:
			mt = new ServerInstruction();
			break;

		default:
			break;
		}
	}

	return mt;
};
