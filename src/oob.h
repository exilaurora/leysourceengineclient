#pragma once

#ifndef DATAGRAM_SIZE
#define DATAGRAM_SIZE 1000000
#endif


/**
 * @file oob.h
 * @brief Обработка out-of-band (OOB) сообщений (запросы/ответы вне основного канала).
 *
 * OOB-пакеты используются для discovery/handshake и простых запросов к серверу
 * без установки полноценного сетевого канала.
 */

class leychan;
class leynet_udp;
class Datagram;
class Steam;

extern unsigned long serverchallenge;
extern unsigned long _ourchallenge;

/** Класс для работы с OOB-пакетами (приём, отправка, парсинг запросов). */
class OOB
{
private:
	OOB();
public:
	OOB(leynet_udp* udp, const char* ip, unsigned short port);
	void Reset();

	bool ReceiveQueryPacketReject(leychan* chan, Datagram* datagram, bf_read& recvdata);
	bool ReceiveQueryPacketGetChallenge(
		leychan* chan,
		bf_read& recvdata,
		const char* nickname,
		const char* password,
		Steam* steam,
		unsigned long* ourchallenge);
	bool ReceiveQueryPacketConnection(leychan* chan, Datagram* datagram, bf_read& recvdata);
	bool ReceiveQueryPacketIgnore(leychan* chan, bf_read& recvdata);

	bool ReceiveQueryPacket(
		leychan* chan,
		Datagram* datagram,
		Steam* steam,
		bf_read& recvdata,
		const char* nickname,
		const char* password,
		int connection_less,
		unsigned long* ourchallenge);
	bool HandleSplitPacket(leychan* chan, bf_read& recvdata, char* netrecbuffer, int msgsize, long* header);
	bool HandleCompressedPacket(leychan* chan, bf_read& recvdata, char* netrecbuffer, int msgsize);
	bool SendRequestChallenge(leychan* chan, long ourchallenge);

private:
	leynet_udp* udp;


	char* oob;

	bf_write* netoob;

	char ip[30];
	unsigned short port;


};
