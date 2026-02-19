#include <string>
#include <stdint.h>
#include <vector>

/**
 * @file commandline.h
 * @brief Утилита для парсинга параметров командной строки.
 *
 * Простой класс-обёртка над массивом аргументов `argv`/`argc`.
 * Позволяет получать числовые и строковые параметры по имени с
 * поддержкой значений по умолчанию.
 */

class CommandLine
{
public:
	CommandLine();

	/** Инициализировать внутренний список параметров из argv. */
	void InitParams(const char* argv[], int argc);

	/** Получить числовой параметр (или defaultvalue если отсутствует). */
	int GetParameterNumber(const char* param, bool optional = false, int defaultvalue = 0);
	/** Получить строковый параметр (или defaultvalue если отсутствует). */
	std::string GetParameterString(const char* param, bool optional = false, std::string defaultvalue = "");

private:
	bool GetParameterInternal(std::string param, std::string* fill, bool optional = false);
	std::vector<std::string> args;
};