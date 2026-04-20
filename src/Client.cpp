#include "../includes/Client.hpp"

Client::Client()
{
    // throw std::runtime_error("Need a fd");
}

Client::Client(int fd) : fd(fd){}

Client::~Client(){}

void Client::setInput(std::string &input)
{
    this->input = input;
}

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

const std::string &Client::getInput()
{
    return (this->input);
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