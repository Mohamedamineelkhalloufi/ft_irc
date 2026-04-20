#include "../includes/Server.hpp"

Server::Server()
{
    throw std::runtime_error("Need a port and password");
}

Server::Server(std::string port, std::string password)
{
    if (isInt(port))
        throw std::runtime_error("Invalid port");
    if (password.size() > 20)
        throw std::runtime_error("Invalid password");
    this->port = std::atoi(port.c_str());
    this->password = password;
    if (this->port < 1024 || this->port > 65535)
        throw std::runtime_error("Invalid port");
}

Server::~Server(){}

void Server::init()
{
    this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->socket_fd < 0)
        throw std::runtime_error("Error socket() failed");

    struct sockaddr_in Serveraddr;
    Serveraddr.sin_family = AF_INET;
    Serveraddr.sin_port = htons(this->port);
    Serveraddr.sin_addr.s_addr = INADDR_ANY;
    
    int rbind = bind(socket_fd, (sockaddr*)&Serveraddr, sizeof(Serveraddr));
    if(rbind < 0)
        throw std::runtime_error("Error bind() failed");

    int rlisten = listen(socket_fd, 10);
    if (rlisten < 0)
        throw std::runtime_error("Error listen() failed");
    std::cout << "listen in port: "<< this->port << std::endl;

    pollfd poll_socket;
    poll_socket.fd = socket_fd;
    poll_socket.events = POLLIN;
    poll_socket.revents = 0;
    
    this->vec_poll.push_back(poll_socket);
}

void Server::run()
{
    while(true)
    {
        int rpoll = poll(vec_poll.data(), vec_poll.size(), -1);
        if (rpoll < 0)
            throw std::runtime_error("Error poll() failed");
        size_t size_vec = vec_poll.size();
        for (size_t i = 0; i < size_vec; i++)
        {
            if(vec_poll[i].revents & POLLIN)
            {
                if (vec_poll[i].fd == socket_fd)
                    acceptClient();
                else
                    handleClient(vec_poll[i].fd);
            }
        }
    }
}

void Server::acceptClient()
{
    int client_fd = accept(socket_fd, 0, 0);
    if (client_fd < 0)
    {
        std::cerr << "Error accept() failed" << std::endl;
        return ;
    }

    this->Clients[client_fd] = Client(client_fd);
    pollfd new_client;
    new_client.fd = client_fd;
    new_client.events = POLLIN;
    new_client.revents = 0;
    
    vec_poll.push_back(new_client);
    std::cout << "Client connected (fd: " << client_fd << ")" << std::endl;}

void Server::handleClient(int fd)
{
    char buffr[1024] = {0};
    recv(fd, buffr, 1023, 0);
     if (buffr[0] == '1')
     {
        for(size_t i = 1; i < this->vec_poll.size(); i++)
            close(this->vec_poll[i].fd);
        close(this->socket_fd);
        std::cout << "exit :)" << std::endl;
        exit(0);
    }
    std::cout << buffr;
}

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