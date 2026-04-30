#ifndef Server_HPP
#define Server_HPP

#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <vector>
#include <cstdlib>
#include <map>
#include <exception>
#include <sstream>
#include <signal.h>
#include <cerrno>
#include <algorithm>
#include "Client.hpp"
#include "Utils.hpp"
#include "Channel.hpp"

class Client;

class Server
{
private:
    int socket_fd;
    int port;
    std::string sport;
    std::string password;
    std::vector<pollfd> vec_poll;

public:
    static bool off;
    std::map<int, Client>          Clients;
    std::map<std::string, Channel> _channels;

    Server();
    Server(std::string port, std::string password);
    ~Server();

    void init();
    void run();
    void acceptClient();
    void handleClient(int fd);
    void disconnected(int fd);
    void checkPassword(int fd, std::istringstream &sstring, std::string &nstring);
    void checkNickname(int fd, std::istringstream &sstring, std::string &nstring);
    void checkUsername(int fd, std::istringstream &sstring, std::string &nstring);
    void isValid(int fd);
    void clean();

    std::string  getPrefix(int fd);
    Channel*     getOrCreateChannel(const std::string& name);
    Client*      getClientByNick(const std::string& nick);
    void         sendToClient(int fd, const std::string& msg);
    void         broadcastToChannel(const std::string& channame, const std::string& msg, int excludeFd = -1);
    void         sendNames(int fd, Channel* chan);

    void cmdJoin(int fd, const std::vector<std::string>& params);
    void cmdPrivmsg(int fd, const std::vector<std::string>& params, const std::string& risala);
    void cmdPart(int fd, const std::vector<std::string>& params, const std::string& risala);
    void cmdKick(int fd, const std::vector<std::string>& params, const std::string& risala);
    void cmdInvite(int fd, const std::vector<std::string>& params);
    void cmdTopic(int fd, const std::vector<std::string>& params, const std::string& risala, bool hasrisala);
    void cmdMode(int fd, const std::vector<std::string>& params);
};

#endif
