#pragma once

/**
 * @file netmsg_common.h
 * @brief Общая обёртка для сетевых сообщений (метаданные сообщения).
 *
 * Содержит имя сообщения, тип и минимально допустимый остаточный размер
 * полезной нагрузки — используется при проверке корректности сообщений.
 */

#include <string>

class netmsg_common
{
private:
	netmsg_common();

public:
	netmsg_common(std::string name, int msgtype, int mininumremainingsize)
	{
		this->name = name;
		this->msgtype = msgtype;
		this->mininumremainingsize = mininumremainingsize;
	}

	/** Имя/идентификатор сообщения. */
	std::string GetName()
	{
		return name;
	}

	/** Тип сообщения (числовой код). */
	int GetMsgType()
	{
		return msgtype;
	}

	/** Минимальный оставшийся размер полезной нагрузки (для проверки). */
	int GetMinimumRemainingSize()
	{
		return mininumremainingsize;
	}

	/** Возвращает true, если длина полезной нагрузки меньше ожидаемой. */
	inline bool LengthTooSmall(int length)
	{
		if (GetMinimumRemainingSize() > length)
		{
			return true;
		}

		return false;
	}
private:
	std::string name;
	int msgtype;
	int mininumremainingsize;
};