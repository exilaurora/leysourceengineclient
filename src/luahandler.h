#pragma once

class leychan;
class bf_read;

// Объявление функции-обработчика, реализованной в luahandler.cpp
int NetHandler(leychan* netchannel, bf_read& msg, int id, char* data);

void RegisterNetworkStrings(char* data);