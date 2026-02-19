#pragma once

/**
 * @file net_messages.h
 * @brief Сборный заголовок для сетевых (net) обработчиков сообщений.
 *
 * Подключает конкретные реализации обработчиков net_*.
 */

#include "net/net_nop.h"
#include "net/net_file.h"
#include "net/net_tick.h"
#include "net/net_stringcmd.h"
#include "net/net_setconvar.h"
#include "net/net_signonstate.h"