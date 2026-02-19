#pragma once

/**
 * @file datagram.h
 * @brief Обёртка для датаграмм/пакетов, использующихся в обмене данными.
 *
 * Класс `Datagram` используется для подготовки и отправки больших
 * блоков данных по UDP (включая запросы фрагментов, реконнекты и
 * генерацию файлов в собственном формате проекта).
 */

#ifndef DATAGRAM_SIZE
#define DATAGRAM_SIZE 1000000
#endif


class leychan;
class leynet_udp;

/**
 * @class Datagram
 * @brief Управление датаграммами, отправка/восстановление фрагментов.
 */
class Datagram
{
private:
	Datagram();
public:
	Datagram(leynet_udp* udp, const char* ip, unsigned short port);

	/** Сбрасывает состояние датаграммы. */
	bool Reset();
	/** Отключает текущий канал (если подключён). */
	bool Disconnect(leychan* chan);
	/** Пытается переподключиться к каналу. */
	bool Reconnect(leychan* chan);
	/** Отправляет датаграмму через указанный канал. */
	bool Send(leychan* chan, bool subchans = false);
	/** Запрашивает недостающие фрагменты у сервера/канала. */
	bool RequestFragments(leychan* chan);
	/** Генерирует файл проекта (внутренний формат) с содержимым. */
	void GenerateLeyFile(leychan* chan, const char* filename, const char* content);



private:
	leynet_udp* udp;


	char* datagram;

	bf_write* netdatagram;

	char ip[30];
	unsigned short port;


};