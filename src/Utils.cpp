#include "../includes/Utils.hpp"

Server *gServer = NULL;

bool isInt(std::string &arg)
{
    size_t i = 0;

    if (i == arg.size())
        return (false);
    while (i < arg.size())
    {
        if(!isdigit(arg[i]))
            return (false);
        i++;
    }
    return (true);
}

void handleSignals(int si)
{
    if (si == SIGINT ||  si == SIGTERM)
    {
        Server::off = false;

        std::cout << "\n--------------------------------------" << std::endl;
        std::cout << "         ircserv : goodbye :) "<< std::endl;
        std::cout << "--------------------------------------" << std::endl;
    }
}
