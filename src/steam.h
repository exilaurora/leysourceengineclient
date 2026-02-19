#include <string>
#include <stdint.h>

/**
 * @file steam.h
 * @brief Минимальный интерфейс для взаимодействия с установленным Steam.
 *
 * Обёртка для получения папки установки Steam и доступа к основным
 * интерфейсам (пользователь, клиент). Используется для интеграции
 * функциональности, завязанной на Steam API.
 */

class ISteamClient017;
class IClientEngine;
class ISteamUser017;

/**
 * @class Steam
 * @brief Упрощённый доступ к Steam API (инициализация и получение пользователя).
 */
class Steam
{
public:
	/** Возвращает путь к установленной папке Steam (если найдено). */
	static std::wstring GetSteamInstallFolder();

	/** Возвращает интерфейс текущего Steam-пользователя. */
	ISteamUser017* GetSteamUser();

	/** Инициализация внутреннего состояния/подключения к Steam. */
	int Initiate();

private:
	ISteamClient017* steamClient;
	ISteamUser017* steamUser;

	int32_t steamPipeHandle;
	int32_t steamUserHandle;

};
