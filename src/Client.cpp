#include "../includes/Client.hpp"

Client::Client(){}

Client::Client(int fd) : fd(fd), valid(false), vUser(false), vNick(false), vPassword(false){}

Client::~Client(){}

void Client::setNickname(std::string &name)
{
    this->nickname = name;
}

void Client::setUsername(std::string &name)
{
    this->username = name;
}

void Client::setValid(bool what)
{
    this->valid = what;
}

void Client::setVuser(bool what)
{
    this->vUser = what;
}

void Client::setVnick(bool what)
{
    this->vNick = what;
}

const std::string &Client::getNickname()
{
    return (this->nickname);
}

const std::string &Client::getUsername()
{
    return (this->username);
}

const int &Client::getFd()
{
    return (this->fd);
}

bool Client::isValid()
{
    return (this->valid);
}

bool Client::isVuser()
{
    return (this->vUser);
}

bool Client::isVnick()
{
    return (this->vNick);
}