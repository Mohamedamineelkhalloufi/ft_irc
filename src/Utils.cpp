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
    if(i != 1)
        return (false);
    return (true);
}

void handleSignals(int si)
{
    if (si == SIGINT ||  si == SIGTERM)
    {
        if(gServer)
        {
            gServer->off = false;
            gServer->clean();
        }
        std::cout << std::endl << "ircserv : goodbye :)" << std::endl;
    }
}