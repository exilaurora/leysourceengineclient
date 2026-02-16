#include "../../valve/buf.h"

#include "../../leychan.h"
#include "svc_setview.h"

bool svc_setview::Register(leychan* chan)
{
	leychan::netcallbackfn fn = reinterpret_cast<leychan::netcallbackfn>(&svc_setview::ParseMessage);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_setview::ParseMessage(leychan* chan, svc_setview* thisptr, bf_read& msg)
{
	int ent = msg.ReadUBitLong(MAX_EDICT_BITS);

	// printf("Received svc_SetView, ent: %i\n", ent);
	return true;
}