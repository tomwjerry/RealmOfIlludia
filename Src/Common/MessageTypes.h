#ifndef MESSAGE_TYPES_H
#define MESSAGE_TYPES_H

#include <string>

enum MessageType {
	MTNone,
	MTServerInstruction
};

class MessageTypes
{
public:
	unsigned char messageType = 0;

	static std::string Serialize(MessageTypes mt);
	static MessageTypes* Parse(std::string data);
};

class ServerInstruction : public MessageTypes
{
public:
	unsigned char instruction;
	void dummyfunc();
};

#endif