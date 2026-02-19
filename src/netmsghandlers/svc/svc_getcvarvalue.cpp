#include "../../valve/buf.h"

#include "../../leychan.h"
#include "../../vector.h"
#include "svc_getcvarvalue.h"

bool svc_getcvarvalue::Register(leychan* chan)
{
	leychan::netcallbackfn fn = reinterpret_cast<leychan::netcallbackfn>(&svc_getcvarvalue::ParseMessage);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_getcvarvalue::ParseMessage(leychan* chan, svc_getcvarvalue* thisptr, bf_read& msg)
{
	int cookie = msg.ReadUBitLong(32);


	char cvarname[255];
	msg.ReadString(cvarname, sizeof(cvarname));

	printf("Received svc_GetCvarValue, cookie: %i | name: %s\n", cookie, cvarname);

	return true;
}