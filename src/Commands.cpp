#include "../includes/Server.hpp"

void Server::cmdJoin(int fd, const std::vector<std::string>& params)
{
	if (params.empty())
	{
		sendToClient(fd, ":ircserv 461 " + Clients[fd].getNickname() + " JOIN :Not enough parameters\r\n");
		return;
	}

	std::vector<std::string> chans, keys;
	std::istringstream chanStream(params[0]);
	std::string token;
	while (std::getline(chanStream, token, ','))
		chans.push_back(token);

	if (params.size() > 1)
	{
		std::istringstream keyStream(params[1]);
		while (std::getline(keyStream, token, ','))
			keys.push_back(token);
	}

	for (size_t i = 0; i < chans.size(); ++i)
	{
		std::string channame = chans[i];
		std::string key;
		if (i < keys.size())
			key = keys[i];
		else
			key = "";

		if (channame.empty() || (channame[0] != '#' && channame[0] != '&'))
		{
			sendToClient(fd, ":ircserv 476 " + Clients[fd].getNickname() + " " + channame + " :Bad channel mask\r\n");
			continue;
		}

		Channel* chan = getOrCreateChannel(channame);
		if (chan->hasMember(fd))
		{
			continue;
		}

		if (chan->isInviteOnly() && !chan->isInvited(fd))
		{
			sendToClient(fd, ":ircserv 473 " + Clients[fd].getNickname() + " " + channame + " :Cannot join channel (+i)\r\n");
			continue;
		}
		if (!chan->getKey().empty() && chan->getKey() != key)
		{
			sendToClient(fd, ":ircserv 475 " + Clients[fd].getNickname() + " " + channame + " :Cannot join channel (+k)\r\n");
			continue;
		}
		if (chan->getUserLimit() > 0 && (int)chan->getMembers().size() >= chan->getUserLimit())
		{
			sendToClient(fd, ":ircserv 471 " + Clients[fd].getNickname() + " " + channame + " :Cannot join channel (+l)\r\n");
			continue;
		}

		chan->addMember(fd);
		chan->removeInvited(fd);
		if (chan->getMembers().size() == 1)
			chan->addOperator(fd);

		broadcastToChannel(channame, ":" + getPrefix(fd) + " JOIN " + channame + "\r\n");

		if (!chan->getTopic().empty())
			sendToClient(fd, ":ircserv 332 " + Clients[fd].getNickname() + " " + channame + " :" + chan->getTopic() + "\r\n");
		else
			sendToClient(fd, ":ircserv 331 " + Clients[fd].getNickname() + " " + channame + " :No topic is set\r\n");

		sendNames(fd, chan);
	}
}


void Server::cmdPrivmsg(int fd, const std::vector<std::string>& params, const std::string& input_)
{
	if (params.empty())
	{
		sendToClient(fd, ":ircserv 411 " + Clients[fd].getNickname() + " :No recipient given (PRIVMSG)\r\n");
		return;
	}
	if (input_.empty())
	{
		sendToClient(fd, ":ircserv 412 " + Clients[fd].getNickname() + " :No text to send\r\n");
		return;
	}

	if (params.size() > 1)
	{
		sendToClient(fd, ":ircserv 407 " + Clients[fd].getNickname() + " :Duplicate recipients\r\n");
		return;
	}

	std::istringstream ss(params[0]);
	std::string target;

	while (std::getline(ss, target, ','))
	{
		if (target == Clients[fd].getNickname())
		{
			sendToClient(fd, ":ircserv 442 " + Clients[fd].getNickname() + " :You can't message yourself\r\n");
			continue;
		}
		std::string msg = ":" + getPrefix(fd) + " PRIVMSG " + target + " :" + input_ + "\r\n";
		if (target[0] == '#' || target[0] == '&')
		{
			if (_channels.find(target) == _channels.end() || !_channels[target].hasMember(fd))
			{
				sendToClient(fd, ":ircserv 442 " + Clients[fd].getNickname() + " " + target + " :You're not on that channel\r\n");
				continue;
			}
			broadcastToChannel(target, msg, fd);
		}
		else
		{
			Client *dest = getClientByNick(target);
			if (!dest || !dest->isValid())
			{
				sendToClient(fd, ":ircserv 401 " + Clients[fd].getNickname() + " " + target + " :No such nick\r\n");
				continue;
			}
			sendToClient(dest->getFd(), msg);
		}
	}
}


void Server::cmdPart(int fd, const std::vector<std::string>& params, const std::string& input_)
{
	if (params.empty())
	{
		sendToClient(fd, ":ircserv 461 " + Clients[fd].getNickname() + " PART :Not enough parameters\r\n");
		return;
	}

	std::istringstream ss(params[0]);
	std::string channame;
	while (std::getline(ss, channame, ','))
	{
		if (_channels.find(channame) == _channels.end() || !_channels[channame].hasMember(fd))
		{
			sendToClient(fd, ":ircserv 442 " + Clients[fd].getNickname() + " " + channame + " :You're not on that channel\r\n");
			continue;
		}

		std::string cose;
		if (input_.empty())
			cose = Clients[fd].getNickname();
		else
			cose = input_;

		broadcastToChannel(channame, ":" + getPrefix(fd) + " PART " + channame + " :" + cose + "\r\n");
		_channels[channame].removeMember(fd);
		if (_channels[channame].isEmpty())
			_channels.erase(channame);
	}
}


void Server::cmdKick(int fd, const std::vector<std::string>& params, const std::string& input_)
{
	if (params.size() < 2)
	{
		sendToClient(fd, ":ircserv 461 " + Clients[fd].getNickname() + " KICK :Not enough parameters\r\n");
		return;
	}

	const std::string& channame = params[0];
	const std::string& dhi   = params[1];

	if (_channels.find(channame) == _channels.end())
	{
		sendToClient(fd, ":ircserv 403 " + Clients[fd].getNickname() + " " + channame + " :No such channel\r\n");
		return;
	}

	Channel &chan = _channels[channame];
	if (!chan.hasMember(fd))
	{
		sendToClient(fd, ":ircserv 442 " + Clients[fd].getNickname() + " " + channame + " :You're not on that channel\r\n");
		return;
	}
	if (!chan.isOperator(fd))
	{
		sendToClient(fd, ":ircserv 482 " + Clients[fd].getNickname() + " " + channame + " :You're not channel operator\r\n");
		return;
	}

	Client* target = getClientByNick(dhi);
	if (!target || !chan.hasMember(target->getFd()))
	{
		sendToClient(fd, ":ircserv 441 " + Clients[fd].getNickname() + " " + dhi + " " + channame + " :They aren't on that channel\r\n");
		return;
	}

	std::string cose;
	if (input_.empty())
		cose = Clients[fd].getNickname();
	else
		cose = input_;

	broadcastToChannel(channame, ":" + getPrefix(fd) + " KICK " + channame + " " + dhi + " :" + cose + "\r\n");
	chan.removeMember(target->getFd());
	if (chan.isEmpty())
		_channels.erase(channame);
}


void Server::cmdInvite(int fd, const std::vector<std::string>& params)
{
	if (params.size() < 2)
	{
		sendToClient(fd, ":ircserv 461 " + Clients[fd].getNickname() + " INVITE :Not enough parameters\r\n");
		return;
	}

	const std::string& dfi    = params[0];
	const std::string& channame = params[1];

	if (_channels.find(channame) == _channels.end())
	{
		sendToClient(fd, ":ircserv 403 " + Clients[fd].getNickname() + " " + channame + " :No such channel\r\n");
		return;
	}

	Channel& chan = _channels[channame];
	if (!chan.hasMember(fd))
	{
		sendToClient(fd, ":ircserv 442 " + Clients[fd].getNickname() + " " + channame + " :You're not on that channel\r\n");
		return;
	}
	if (chan.isInviteOnly() && !chan.isOperator(fd))
	{
		sendToClient(fd, ":ircserv 482 " + Clients[fd].getNickname() + " " + channame + " :You're not channel operator\r\n");
		return;
	}

	Client* target = getClientByNick(dfi);
	if (!target)
	{
		sendToClient(fd, ":ircserv 401 " + Clients[fd].getNickname() + " " + dfi + " :No such nick\r\n");
		return;
	}
	if (chan.hasMember(target->getFd()))
	{
		sendToClient(fd, ":ircserv 443 " + Clients[fd].getNickname() + " " + dfi + " " + channame + " :is already on channel\r\n");
		return;
	}

	chan.addInvited(target->getFd());
	sendToClient(fd, ":ircserv 341 " + Clients[fd].getNickname() + " " + dfi + " " + channame + "\r\n");
	sendToClient(target->getFd(), ":" + getPrefix(fd) + " INVITE " + dfi + " :" + channame + "\r\n");
}


void Server::cmdTopic(int fd, const std::vector<std::string>& params, const std::string& input_, bool hasinput_)
{
	if (params.empty())
	{
		sendToClient(fd, ":ircserv 461 " + Clients[fd].getNickname() + " TOPIC :Not enough parameters\r\n");
		return;
	}

	const std::string& channame = params[0];
	if (_channels.find(channame) == _channels.end())
	{
		sendToClient(fd, ":ircserv 403 " + Clients[fd].getNickname() + " " + channame + " :No such channel\r\n");
		return;
	}

	Channel &chan = _channels[channame];
	if (!chan.hasMember(fd))
	{
		sendToClient(fd, ":ircserv 442 " + Clients[fd].getNickname() + " " + channame + " :You're not on that channel\r\n");
		return;
	}

	if (!hasinput_)
	{
		if (chan.getTopic().empty())
			sendToClient(fd, ":ircserv 331 " + Clients[fd].getNickname() + " " + channame + " :No topic is set\r\n");
		else
			sendToClient(fd, ":ircserv 332 " + Clients[fd].getNickname() + " " + channame + " :" + chan.getTopic() + "\r\n");
		return;
	}

	if (chan.isTopicRestricted() && !chan.isOperator(fd))
	{
		sendToClient(fd, ":ircserv 482 " + Clients[fd].getNickname() + " " + channame + " :You're not channel operator\r\n");
		return;
	}

	chan.setTopic(input_);
	broadcastToChannel(channame, ":" + getPrefix(fd) + " TOPIC " + channame + " :" + input_ + "\r\n");
}
