#include "../includes/Channel.hpp"

Channel::Channel() : inviteOnly(false), topicRestricted(false), userLimit(0){}

Channel::Channel(const std::string& name) : name(name), inviteOnly(false), topicRestricted(false), userLimit(0){}

Channel::~Channel() {}

const std::string& Channel::getName() const 
{ 
    return name; 
}

const std::string& Channel::getTopic() const 
{ 
    return topic; 
}

const std::string& Channel::getKey() const
{ 
    return key; 
}

bool Channel::isInviteOnly() const 
{
    return inviteOnly; 
}

bool Channel::isTopicRestricted() const 
{
    return topicRestricted; 
}

int Channel::getUserLimit() const 
{
    return userLimit; 
}

const std::vector<int>& Channel::getMembers() const 
{ 
    return members; 
}

void Channel::addMember(int fd)    
{ 
    if (!hasMember(fd)) members.push_back(fd); 
}

void Channel::removeMember(int fd) 
{
    members.erase(std::remove(members.begin(), members.end(), fd), members.end());
    removeOperator(fd); removeInvited(fd);
}

bool Channel::hasMember(int fd) const 
{
    return std::find(members.begin(), members.end(), fd) != members.end();
}

void Channel::addOperator(int fd)    
{ 
    if (!isOperator(fd)) operators.push_back(fd); 
}

void Channel::removeOperator(int fd) 
{
    operators.erase(std::remove(operators.begin(), operators.end(), fd), operators.end());
}

bool Channel::isOperator(int fd) const 
{
    return std::find(operators.begin(), operators.end(), fd) != operators.end();
}

void Channel::addInvited(int fd)    
{ 
    if (!isInvited(fd)) invitedFds.push_back(fd); 
}

void Channel::removeInvited(int fd) 
{
    invitedFds.erase(std::remove(invitedFds.begin(), invitedFds.end(), fd), invitedFds.end());
}

bool Channel::isInvited(int fd) const 
{
    return std::find(invitedFds.begin(), invitedFds.end(), fd) != invitedFds.end();
}

void Channel::setTopic(const std::string& t) 
{ 
    topic = t; 
}

void Channel::setKey(const std::string& k)   
{ 
    key = k; 
}

void Channel::setInviteOnly(bool v)          
{
    inviteOnly = v; 
}

void Channel::setTopicRestricted(bool v)     
{
    topicRestricted = v; 
}

void Channel::setUserLimit(int l)            
{
    userLimit = l; 
}

bool Channel::isEmpty() const                
{
    return members.empty(); 
}

std::string Channel::getModeString() const 
{
    std::string modes = "+", params;
    if (inviteOnly)      modes += "i";
    if (topicRestricted) modes += "t";
    if (!key.empty())  
    { 
        modes += "k"; params += " " + key; 
    }
    if (userLimit > 0) 
    {
        modes += "l";
        std::ostringstream oss; oss << userLimit;
        params += " " + oss.str();
    }
    if (modes == "+") 
        return "";
    return modes + params;
}