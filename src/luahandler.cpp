#include <lua.hpp>
#include <iostream>
// #include <filesystem>
#include <fstream>
#include <string>

#include "valve/buf.h"
#include "leychan.h"
#include "luahandler.h"

leychan* netchannel;
// Lua C-функция: получает привязанный bf_write* через upvalue (lightuserdata)
static int l_write_string(lua_State* L)
{
    bf_write* sendbuffer = netchannel->GetSendData();
    const char* string = luaL_checkstring(L, 1);
    if (sendbuffer)
        sendbuffer->WriteString(string);
    else
        std::cout << "Error: sendbuffer is null in WriteString" << std::endl;
    return 0;
}

static int l_write_ubitlong(lua_State* L)
{
    bf_write* sendbuffer = netchannel->GetSendData();
    unsigned long data = (unsigned long)luaL_checkinteger(L, 1);
    int numbits = (int)luaL_checkinteger(L, 2);
    if (sendbuffer)
        sendbuffer->WriteUBitLong(data, numbits);
    else
        std::cout << "Error: sendbuffer is null in WriteUBitLong" << std::endl;
    return 0;
}

lua_State *L;

void RegisterNetworkStrings(char* data) {
    printf("%s\n", data);
}

int NetHandler(leychan* netchan, bf_read& msg, int id, char* data)
{
    netchannel = netchan;
    bf_write* sendbuffer = netchannel->GetSendData();

    if (L == nullptr) { // Инициализируем Lua только при первом вызове
        L = luaL_newstate();
        luaL_openlibs(L);
    
        std::ifstream file("handler.lua");
        if (!file) {
            std::cerr << "Can't open handler.lua" << std::endl;
            return 1;
        }

        std::string s((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        lua_pushnumber(L, id);
        lua_setglobal(L, "net_id");

        lua_pushstring(L, data);
        lua_setglobal(L, "net_data");

        // lua_pushlightuserdata(L, sendbuffer);
        lua_pushcclosure(L, l_write_string, 1);
        lua_setglobal(L, "WriteString");

        // lua_pushlightuserdata(L, sendbuffer);
        lua_pushcclosure(L, l_write_ubitlong, 1);
        lua_setglobal(L, "WriteUBitLong");

    

        if (luaL_dostring(L, s.c_str()))
        {
            std::cout << "Error: " << lua_tostring(L, -1) << "\n";
        }
    }
    // Попробуем вызвать функцию HandleNet(id, data), если она определена в скрипте
    lua_getglobal(L, "HandleNet");
    if (lua_isfunction(L, -1))
    {
        // Параметр 1: id
        lua_pushinteger(L, id);

        // Параметр 2: data (передаём как бинарную строку с длиной)
        int data_len = msg.GetNumBytesLeft();
        if (data_len <= 0)
        {
            // Если длина не известна, используем strlen (поддерживает только C-строки)
            lua_pushstring(L, data);
        }
        else
        {
            lua_pushlstring(L, data, data_len);
        }

        // Вызов функции: 2 аргумента, 0 результатов
        if (lua_pcall(L, 2, 0, 0) != LUA_OK)
        {
            std::cerr << "Error calling HandleNet: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1); // удалить сообщение об ошибке
        }
    }
    else
    {
        lua_pop(L, 1); // убрать не-функцию из стека
    }

    // lua_close(L); // Не закрываем Lua, чтобы сохранить состояние между вызовами
    return 0;
}
