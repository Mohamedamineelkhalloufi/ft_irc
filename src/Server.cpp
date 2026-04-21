#include "../includes/Server.hpp"

Server::Server()
{
    throw std::runtime_error("Need a port and password");
}

Server::Server(std::string port, std::string password) : off(true)
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
    
    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
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
    while(this->off)
    {
        int rpoll = poll(vec_poll.data(), vec_poll.size(), -1);
        if (rpoll < 0)
        {
            if (errno == EINTR)
                continue ;
            throw std::runtime_error("Error poll() failed");
        }
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
    char buffer[1024] = {0};
    int rrecv = recv(fd, buffer, 1023, 0);
    if (rrecv <= 0)
        disconnected(fd);
    else if (rrecv > 0)
    {
        buffer[rrecv] = '\0';
        this->Clients[fd].input = buffer;
        std::istringstream sstring(this->Clients[fd].input);
        std::string nstring;
        sstring >> nstring;
        checkPassword(fd, sstring, nstring);
        checkNickname(fd, sstring, nstring);
        checkUsername(fd, sstring, nstring);
        if (!Clients[fd].vPassword)
            send(fd, "464 :Password required\r\n", 25, 0);
        isValid(fd);
    }
    if (buffer[0] == '1')
    {
        for(size_t i = 1; i < this->vec_poll.size(); i++)
            close(this->vec_poll[i].fd);
        close(this->socket_fd);
        std::cout << "exit :)" << std::endl;
        exit(0);
    }
}

void Server::disconnected(int fd)
{
    close(fd);
    Clients.erase(fd);
    for (size_t i = 0; i < vec_poll.size(); i++)
    {
        if (fd == vec_poll[i].fd)
        {
            vec_poll.erase(vec_poll.begin() + i);
            break;
        }
    }
    std::cout << "Client disconnected (fd: " << fd << ")" << std::endl;
}

void Server::checkUsername(int fd, std::istringstream &sstring, std::string &nstring)
{
    std::string check;
    if (nstring == "USER" && Clients[fd].vPassword)
    {
        if (!(sstring >> nstring))
        {
            send(fd, "461 :Not enough parameters\n", 28, 0);
            return ;
        }
        if ((sstring >> check))
        {
            send(fd, "461 :Too many parameters\n", 26, 0);
            return ;
        }
        if (!Clients[fd].isVuser())
        {
            Clients[fd].setVuser(true);
            Clients[fd].setUsername(nstring);
            send(fd, "USER : OK\n", 10, 0);
        }
        else
            send(fd, "462 :You cannot reassign it\n", 29, 0);
    }
}

void Server::checkNickname(int fd, std::istringstream &sstring, std::string &nstring)
{
    std::string check;
    if (nstring == "NICK" && Clients[fd].vPassword)
    {
        if (!(sstring >> nstring))
        {
            send(fd, "461 :Not enough parameters\n", 28, 0);
            return ;
        }
        if ((sstring >> check))
        {
            send(fd, "461 :Too many parameters\n", 26, 0);
            return ;
        }
        for (size_t i = 0; i < Clients.size(); i++)
        {
            if ((int)i != fd && nstring == Clients[i].getNickname())
            {
                send(fd, "433 :Nickname is already in use\n", 33, 0);
                return ;
            }
        }
        if (!Clients[fd].isVnick())
            Clients[fd].setVnick(true);
        Clients[fd].setNickname(nstring);
        send(fd, "NICK : OK\n", 11, 0);
    }
}

void Server::isValid(int fd)
{
    if (Clients[fd].isVnick() && Clients[fd].isVuser())
    {
        Clients[fd].setValid(true);
        send(fd, "001 :Welcome to IRC\n", 21, 0);
    }
}

void Server::checkPassword(int fd, std::istringstream &sstring, std::string &nstring)
{
    std::string check;

    if (nstring == "PASS" && !Clients[fd].vPassword)
    {
        if (!(sstring >> nstring))
            return ;
        if (sstring >> check)
            return ;
        if (nstring == this->password)
        {
            Clients[fd].vPassword = true;
            send(fd, "Password : OK\n", 15, 0);
        }
        else
            send(fd, "464 :Password incorrect\n", 25, 0);
    }
    else if (nstring == "PASS" && Clients[fd].vPassword)
        send(fd, "462 :You may not reregister\n", 29, 0);
}

void Server::clean()
{
    for(size_t i = 1; i < this->vec_poll.size(); i++)
        close(this->vec_poll[i].fd);
    close(this->socket_fd);
}