#pragma once

/**
 * @file tickdata.h
 * @brief Небольшая структура для хранения информации о сетевых тиках.
 */

struct tickData_s
{
	long net_tick;                  ///< Номер/метка сетевого тика
	short net_hostframetime;        ///< Время фрейма на хосте (миллисек.)
	short net_hostframedeviation;   ///< Отклонение времени кадра
};