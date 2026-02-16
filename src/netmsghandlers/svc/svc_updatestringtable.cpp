#include "../../valve/buf.h"

#include "../../leychan.h"
#include "svc_updatestringtable.h"

bool svc_updatestringtable::Register(leychan* chan)
{
	leychan::netcallbackfn fn = reinterpret_cast<leychan::netcallbackfn>(&svc_updatestringtable::ParseMessage);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_updatestringtable::ParseMessage(leychan* chan, svc_updatestringtable* thisptr, bf_read& msg)
{
	int tableid = msg.ReadUBitLong((int)MAX_TABLES_BITS);

	int changed = 1;

	if (msg.ReadOneBit() != 0)
	{
		changed = msg.ReadWord();
	}

	int bits = msg.ReadUBitLong(20);

	if (bits < 1)
		return true;

	char* data = new char[bits];

	msg.ReadBits(data, bits);

	delete[] data;


	// printf("Received svc_UpdateStringTable, id: %i | changed: %i | bits: %i\n", tableid, changed, bits);

	return true;
}