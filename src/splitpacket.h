#pragma once

/**
 * @file splitpacket.h
 * @brief Логика сборки/хранения фрагментированных (split) пакетов.
 *
 * Используется для приёма частей больших сообщений, их накопления
 * и последующей сборки в единый пакет.
 */

#include <memory>
#include <vector>
#include <string>
#include <time.h>
#include <cstring>


/** Одна часть (фрагмент) разбитого пакета. */
class SplitPacketPart
{
public:
	int length;
	int offset;
	std::string data;

	SplitPacketPart()
	{
		length = 0;
		offset = 0;
		data = "";
	}

};

/** Класс-коллектор для частей пакета и сборки полного сообщения. */
class SplitPacket
{
private:
	std::vector<SplitPacketPart> parts;
public:
	SplitPacket()
	{
		parts.clear();
		updateTime = 0;
		sequenceNumber = 0;
		totalExpectedSize = 0;
		expectedPartsCount = 0;
	}
	unsigned int updateTime;        ///< Время последнего обновления (time_t)
	int sequenceNumber;            ///< Номер последовательности пакета
	int totalExpectedSize;         ///< Ожидаемый полный размер в байтах
	int expectedPartsCount;        ///< Ожидаемое число частей

	int GetPartsCount()
	{
		return parts.size();
	}

	int GetCurrentPacketSize()
	{
		int size = 0;

		if (parts.size() > 0)
		{
			for (const auto data : parts)
			{
				size = size + data.length;
			}

		}

		return size;
	}

	bool HasProperParts()
	{
		return GetPartsCount() == expectedPartsCount && expectedPartsCount > 0;
	}

	bool HasProperSize()
	{
		return GetCurrentPacketSize() == totalExpectedSize && totalExpectedSize > 0;
	}

	bool IsComplete()
	{
		return HasProperParts() && HasProperSize();
	}

	/** Вставляет часть в коллекцию. */
	void InsertPart(int offset, int length, char* data)
	{
		SplitPacketPart  newPart;
		newPart.length = length;
		newPart.offset = offset;
		for (int i = 0; i < length; i++)
		{
			newPart.data.push_back(data[i]);
		}

		parts.push_back(newPart);
		this->updateTime = (unsigned int)time(NULL);
	}

	/** Устарел ли пакет (более 10 сек с последнего обновления). */
	bool IsOld()
	{
		return this->updateTime > 0 && (((unsigned int)time(NULL) - this->updateTime) > 10);
	}

	/** Если пакет собран полностью — возвращает новый буфер с данными (надо удалить вручную). */
	char* CreateAssembledPacket()
	{
		if (!IsComplete())
			return 0;

		char* pkg = new char[totalExpectedSize];
		memset(pkg, 0, totalExpectedSize);

		for (auto data : parts)
		{
			memcpy(pkg + data.offset, data.data.c_str(), data.length);
		}

		return pkg;
	}

	/** Сброс состояния и освобождение частей. */
	void Reset()
	{
		updateTime = 0;
		sequenceNumber = 0;
		totalExpectedSize = 0;
		expectedPartsCount = 0;

		for (auto data : parts)
		{
			data.data = "";
		}

		if (parts.empty())
			return;

		parts.clear();
	}
};