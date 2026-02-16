#include "../../valve/buf.h"

#include "../../leychan.h"
#include "net_stringcmd.h"

bool net_stringcmd::Register(leychan* chan)
{
	leychan::netcallbackfn fn = reinterpret_cast<leychan::netcallbackfn>(&net_stringcmd::ParseMessage);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool net_stringcmd::ParseMessage(leychan* chan, net_stringcmd* thisptr, bf_read& msg)
{
	char cmd[1024];
	msg.ReadString(cmd, sizeof(cmd));

	printf("net_StringCmd: %s\n", cmd);
	return true;
}