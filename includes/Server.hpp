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
#include "Client.hpp"
#include "Utils.hpp"

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
    bool off;
    std::map<int, Client> Clients;
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
};

#endif