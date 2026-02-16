#include "../../valve/buf.h"

#include "../../leychan.h"
#include "svc_crosshairangle.h"

bool svc_crosshairangle::Register(leychan* chan)
{
	leychan::netcallbackfn fn = reinterpret_cast<leychan::netcallbackfn>(&svc_crosshairangle::ParseMessage);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_crosshairangle::ParseMessage(leychan* chan, svc_crosshairangle* thisptr, bf_read& msg)
{
	int p = msg.ReadUBitLong(16);
	int y = msg.ReadUBitLong(16);
	int r = msg.ReadUBitLong(16);

	// printf("Received svc_CrosshairAngle p: %d y: %d r: %d\n", p, y, r);

	return true;
}