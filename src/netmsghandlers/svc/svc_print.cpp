#include "../../valve/buf.h"

#include "../../leychan.h"
#include "svc_print.h"

bool svc_print::Register(leychan* chan)
{
	leychan::netcallbackfn fn = reinterpret_cast<leychan::netcallbackfn>(&svc_print::ParseMessage);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_print::ParseMessage(leychan* chan, svc_print* thisptr, bf_read& msg)
{

	char print[2048];
	msg.ReadString(print, sizeof(print));
	// printf("svc_Print:\n");
	// printf("%s", print);

	return true;
}