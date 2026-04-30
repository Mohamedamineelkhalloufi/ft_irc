#include "../includes/Server.hpp"

void Server::cmdMode(int fd, const std::vector<std::string>& params)
{
	if (params.empty())
	{
		sendToClient(fd, ":ircserv 461 " + Clients[fd].getNickname() + " MODE :Not enough parameters\r\n");
		return;
	}

	const std::string& target = params[0];

	if (target[0] != '#' && target[0] != '&')
	{
		if (target != Clients[fd].getNickname())
		{
			sendToClient(fd, ":ircserv 502 " + Clients[fd].getNickname() + " :Cannot change mode for other users\r\n");
			return;
		}
		if (params.size() == 1)
			sendToClient(fd, ":ircserv 221 " + Clients[fd].getNickname() + " +\r\n");
		return;
	}

	if (_channels.find(target) == _channels.end())
	{
		sendToClient(fd, ":ircserv 403 " + Clients[fd].getNickname() + " " + target + " :No such channel\r\n");
		return;
	}

	Channel& chan = _channels[target];

	if (params.size() == 1)
	{
		sendToClient(fd, ":ircserv 324 " + Clients[fd].getNickname() + " " + target + " " + chan.getModeString() + "\r\n");
		return;
	}

	if (!chan.isOperator(fd))
	{
		sendToClient(fd, ":ircserv 482 " + Clients[fd].getNickname() + " " + target + " :You're not channel operator\r\n");
		return;
	}

	const std::string& modeStr = params[1];
	bool adding = true;
	size_t argIdx = 2;
	std::string all_mods;
	std::string all_args;

	for (size_t i = 0; i < modeStr.size(); ++i)
	{
		char mode = modeStr[i];

		if (mode == '+')
		{
			adding = true;
			continue;
		}
		if (mode == '-')
		{
			adding = false;
			continue;
		}

		switch (mode)
		{
			case 'i':
				chan.setInviteOnly(adding);
				if (adding)
					all_mods += "+i";
				else
					all_mods += "-i";
				break;

			case 't':
				chan.setTopicRestricted(adding);
				if (adding)
					all_mods += "+t";
				else
					all_mods += "-t";
				break;

			case 'k':
				if (adding)
				{
					if (argIdx >= params.size())
					{
						sendToClient(fd, ":ircserv 461 " + Clients[fd].getNickname() + " MODE :Not enough parameters\r\n");
						break;
					}
					chan.setKey(params[argIdx]);
					all_args += " " + params[argIdx];
					++argIdx;
					all_mods += "+k";
				}
				else
				{
					chan.setKey("");
					all_mods += "-k";
				}
				break;

			case 'o':
			{
				if (argIdx >= params.size())
				{
					sendToClient(fd, ":ircserv 461 " + Clients[fd].getNickname() + " MODE :Not enough parameters\r\n");
					break;
				}
				Client* opTarget = getClientByNick(params[argIdx]);
				++argIdx;
				if (!opTarget || !chan.hasMember(opTarget->getFd()))
				{
					std::string nick;
					if (opTarget)
						nick = opTarget->getNickname();
					else
						nick = "?";
					sendToClient(fd, ":ircserv 441 " + Clients[fd].getNickname() + " " +
								 nick + " " + target + " :They aren't on that channel\r\n");
					break;
				}
				if (adding)
				{
					chan.addOperator(opTarget->getFd());
					all_mods += "+o";
				}
				else
				{
					chan.removeOperator(opTarget->getFd());
					all_mods += "-o";
				}
				all_args += " " + opTarget->getNickname();
				break;
			}

			case 'l':
				if (adding)
				{
					if (argIdx >= params.size())
					{
						sendToClient(fd, ":ircserv 461 " + Clients[fd].getNickname() + " MODE :Not enough parameters\r\n");
						break;
					}
					std::string num_ = params[argIdx];
					int lim = std::atoi(params[argIdx].c_str());
					++argIdx;
					if (lim <= 0 || !isInt(num_))
					{
						sendToClient(fd, ":ircserv 461 " + Clients[fd].getNickname() + " MODE :Invalid limit\r\n");
						break;
					}
					chan.setUserLimit(lim);
					std::ostringstream oss;
					oss << lim;
					all_args += " " + oss.str();
					all_mods += "+l";
				}
				else
				{
					chan.setUserLimit(0);
					all_mods += "-l";
				}
				break;

			default:
				sendToClient(fd, ":ircserv 472 " + Clients[fd].getNickname() + " " + std::string(1, mode) + " :is unknown mode char to me\r\n");
		}
	}

	if (!all_mods.empty())
		broadcastToChannel(target, ":" + getPrefix(fd) + " MODE " + target + " " + all_mods + all_args + "\r\n");
}