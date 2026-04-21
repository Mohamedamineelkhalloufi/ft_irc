#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"

class Client
{
private:
    int fd;
    std::string username;
    std::string nickname;
    bool valid;
    bool vUser;
    bool vNick;
    
public:
    std::string input;
    bool vPassword;
    Client();
    Client(int fd);
    ~Client();

    bool isValid();
    bool isVuser();
    bool isVnick();
    const std::string &getUsername();
    const std::string &getNickname();
    const int &getFd();
    void setUsername(std::string &name);
    void setNickname(std::string &name);
    void setValid(bool what);
    void setVuser(bool what);
    void setVnick(bool what);
};

#endif