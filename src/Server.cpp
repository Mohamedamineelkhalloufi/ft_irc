#include "../includes/Server.hpp"

bool Server::off = true;

Server::Server()
{
    throw std::runtime_error("Error: Need a port and password");
}

Server::Server(std::string port, std::string password)
{
    size_t to = password.find(" ");
    if (to != std::string::npos)
        throw std::runtime_error("Error: Password cannot contain spaces");
    if (!isInt(port))
        throw std::runtime_error("Error: Invalid port");
    if (password.size() > 20)
        throw std::runtime_error("Error: Password too long");
    this->port = std::atoi(port.c_str());
    this->password = password;
    if (this->port < 1024 || this->port > 65535)
        throw std::runtime_error("Error: Port out of range (1024–65535)");
}

Server::~Server(){}

void Server::init()
{
    this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->socket_fd < 0)
        throw std::runtime_error("Error: socket() failed");

    struct sockaddr_in Serveraddr;
    Serveraddr.sin_family = AF_INET;
    Serveraddr.sin_port = htons(this->port);
    Serveraddr.sin_addr.s_addr = INADDR_ANY;
    
    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    int rbind = bind(socket_fd, (sockaddr*)&Serveraddr, sizeof(Serveraddr));
    if(rbind < 0)
        throw std::runtime_error("Error: bind() failed");

    int rlisten = listen(socket_fd, 10);
    if (rlisten < 0)
        throw std::runtime_error("Error: listen() failed");
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "   ircserv listening on port " << this->port << std::endl;
    std::cout << "--------------------------------------" << std::endl;
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
            throw std::runtime_error("Error: poll() failed");
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
    this->clean();
}

void Server::acceptClient()
{
    int client_fd = accept(socket_fd, 0, 0);
    if (client_fd < 0)
    {
        std::cerr << "Error: accept() failed" << std::endl;
        return ;
    }

    this->Clients[client_fd] = Client(client_fd);
    pollfd new_client;
    new_client.fd = client_fd;
    new_client.events = POLLIN;
    new_client.revents = 0;
    
    vec_poll.push_back(new_client);
    std::cout << "Client connected (fd: " << client_fd << ")" << std::endl;
}

void Server::handleClient(int fd)
{
    char buffer[1024] = {0};
    int rrecv = recv(fd, buffer, 1023, 0);
    if (rrecv <= 0)
        disconnected(fd);
    else if (rrecv > 0)
    {
        buffer[rrecv] = '\0';
        this->Clients[fd].input += buffer;
        size_t to;
        while ((to = Clients[fd].input.find("\r\n")) != std::string::npos)
        {
            std::string line = Clients[fd].input.substr(0, to);
        
            Clients[fd].input.erase(0, to + 1);
 
            std::istringstream sstring(line);
            std::string nstring;
            sstring >> nstring;
            checkPassword(fd, sstring, nstring);
            checkNickname(fd, sstring, nstring);
            checkUsername(fd, sstring, nstring);
            isValid(fd);
            if (Clients[fd].isValid())
            {
                std::vector<std::string> params;
                std::string msg;
                bool hasmsg = false;
                std::string token;
                while (sstring >> token)
                {
                    if (token[0] == ':')
                    {
                        hasmsg = true;
                        msg = token.substr(1);
                        std::string g_msg;
                        if (std::getline(sstring, g_msg))
                            msg += g_msg;
                        break;
                    }
                    params.push_back(token);
                }
                if      (nstring == "JOIN")    
                    cmdJoin(fd, params);
                else if (nstring == "PRIVMSG") 
                    cmdPrivmsg(fd, params, msg);
                else if (nstring == "PART")    
                    cmdPart(fd, params, msg);
                else if (nstring == "KICK")    
                    cmdKick(fd, params, msg);
                else if (nstring == "INVITE")  
                    cmdInvite(fd, params);
                else if (nstring == "TOPIC")   
                    cmdTopic(fd, params, msg, hasmsg);
                else if (nstring == "MODE")    
                    cmdMode(fd, params);
            }
        }
    }
}

void Server::disconnected(int fd)
{
    std::vector<std::string> lshrem;
        for (std::map<std::string, Channel>::iterator it = _channels.begin();
             it != _channels.end(); ++it)
        {
            if (it->second.hasMember(fd))
            {
                it->second.removeMember(fd);
                if (it->second.isEmpty())
                    lshrem.push_back(it->first);
            }
        }
    for (size_t i = 0; i < lshrem.size(); i++)
            _channels.erase(lshrem[i]);
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

void Server::checkPassword(int fd, std::istringstream &sstring, std::string &nstring)
{
    std::string check;
    
    if (nstring == "PASS" && !Clients[fd].vPassword)
    {
        if (!(sstring >> nstring))
            send(fd, "461 :Not enough parameters\n", 28, 0);
        else if (sstring >> check)
            send(fd, "461 :Too many parameters\n", 26, 0);
        else if (nstring == this->password)
        {
            Clients[fd].vPassword = true;
            send(fd, "Password : OK\n", 15, 0);
        }
        else
            send(fd, "464 :Password incorrect\n", 25, 0);
    }
    else if (nstring == "PASS" && Clients[fd].vPassword)
        send(fd, "462 :You may not reregister\n", 29, 0);
    else if (!Clients[fd].vPassword)
        send(fd, "464 :Password required\n", 24, 0);
}

void Server::isValid(int fd)
{
    if (Clients[fd].isVnick() && Clients[fd].isVuser() && !Clients[fd].isValid())
    {
        Clients[fd].setValid(true);
        send(fd, "001 :Welcome to IRC\n", 21, 0);
    }
}

void Server::clean()
{
    for(size_t i = 1; i < this->vec_poll.size(); i++)
        close(this->vec_poll[i].fd);
    close(this->socket_fd);
}

std::string Server::getPrefix(int fd)
{
	return Clients[fd].getNickname() + "!" + Clients[fd].getUsername() + "@localhost";
}

Channel* Server::getOrCreateChannel(const std::string& name)
{
	if (_channels.find(name) == _channels.end())
		_channels.insert(std::make_pair(name, Channel(name)));
	return &_channels.at(name);
}

Client* Server::getClientByNick(const std::string& nick)
{
	for (std::map<int, Client>::iterator it = Clients.begin(); it != Clients.end(); ++it)
		if (it->second.getNickname() == nick)
			return &it->second;
	return NULL;
}

void Server::sendToClient(int fd, const std::string& msg)
{
	send(fd, msg.c_str(), msg.size(), 0);
}

void Server::broadcastToChannel(const std::string& channame, const std::string& msg, int mol_fd)
{
	if (_channels.find(channame) == _channels.end())
		return;
	const std::vector<int>& members = _channels.at(channame).getMembers();
	for (size_t i = 0; i < members.size(); ++i)
		if (members[i] != mol_fd)
			sendToClient(members[i], msg);
}

void Server::sendNames(int fd, Channel* chan)
{
	std::string list;
	const std::vector<int>& members = chan->getMembers();

	for (size_t i = 0; i < members.size(); ++i)
	{
		if (Clients.find(members[i]) == Clients.end())
		{
			continue;
		}

		if (chan->isOperator(members[i])) list += "@";
		list += Clients[members[i]].getNickname();
		if (i + 1 < members.size()) list += " ";
	}
	std::string nick = Clients[fd].getNickname();
	sendToClient(fd, ":ircserv 353 " + nick + " = " + chan->getName() + " :" + list + "\r\n");
	sendToClient(fd, ":ircserv 366 " + nick + " " + chan->getName() + " :End of /NAMES list\r\n");
}