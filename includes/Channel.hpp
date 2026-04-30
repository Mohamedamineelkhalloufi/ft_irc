#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

class Channel
{
private:
    std::string      name;
    std::string      topic;
    std::string      key;
    std::vector<int> members;
    std::vector<int> operators;
    std::vector<int> invitedFds;
    bool             inviteOnly;
    bool             topicRestricted;
    int              userLimit;

public:
    Channel();
    Channel(const std::string& name);
    ~Channel();

    const std::string&      getName()           const;
    const std::string&      getTopic()          const;
    const std::string&      getKey()            const;
    bool                    isInviteOnly()      const;
    bool                    isTopicRestricted() const;
    int                     getUserLimit()      const;
    const std::vector<int>& getMembers()        const;

    void addMember(int fd);
    void removeMember(int fd);
    bool hasMember(int fd)   const;

    void addOperator(int fd);
    void removeOperator(int fd);
    bool isOperator(int fd)  const;

    void addInvited(int fd);
    void removeInvited(int fd);
    bool isInvited(int fd)   const;

    void setTopic(const std::string& t);
    void setKey(const std::string& k);
    void setInviteOnly(bool v);
    void setTopicRestricted(bool v);
    void setUserLimit(int l);

    bool        isEmpty()       const;
    std::string getModeString() const;
};

#endif