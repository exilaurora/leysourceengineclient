#include "../../leychan.h"

#include "net_nop.h"

bool net_nop::Register(leychan* chan)
{
	leychan::netcallbackfn fn = reinterpret_cast<leychan::netcallbackfn>(&net_nop::ParseMessage);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool net_nop::ParseMessage(leychan* chan, net_nop* thisptr, bf_read& msg)
{

	return true;
}