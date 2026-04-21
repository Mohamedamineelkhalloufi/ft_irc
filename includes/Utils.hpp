#ifndef UTILS_HPP
#define UTILS_HPP

#include "Server.hpp"

class Server;
extern Server *gServer;

bool isInt(std::string &arg);
void handleSignals(int kill);

#endif