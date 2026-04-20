#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"

class Client
{
private:
    int fd;
    std::string username;
    std::string nickname;
    std::string input;
    bool valid;

public:
    Client();
    Client(int fd);
    ~Client();

    bool isValid();
    const std::string &getUsername();
    const std::string &getNickname();
    const std::string &getInput();
    const int &getFd();
    void setUsername(std::string &name);
    void setNickname(std::string &name);
    void setInput(std::string &input);
    void setValid(bool what);
};

#endif