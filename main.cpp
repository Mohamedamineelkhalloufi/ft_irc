#include "includes/Server.hpp"

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cout << "Usage : " << av[0] << " <PORT> && <PASSWORD>" << std::endl;
        return (1);
    }
    try
    {
        Server irc(av[1], av[2]);
    
        gServer = &irc;
        signal(SIGINT, handleSignals);
        signal(SIGTERM, handleSignals);
        irc.init();
        irc.run();
    }
    catch(std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return (1);
    }
    return (0);
}