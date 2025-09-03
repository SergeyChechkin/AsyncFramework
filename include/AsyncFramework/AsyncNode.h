#pragma once

#include "Threads/SingleThread.h"
#include "common/macros.h"

#include <memory> 
#include <unordered_map> 
#include <unordered_set> 
#include <functional>


// Abstract class for message
class MessageBase {
public:    
};

class AsyncSubscriberBase {
public:
    virtual void writeMessage(const std::shared_ptr<MessageBase>& msg) = 0;
};

class AsyncRequestResponseBase {
public:
    virtual void writeMessage(const std::shared_ptr<MessageBase>& msg) = 0;
};


class AsyncSystem {
public:
    static std::shared_ptr<AsyncSystem> getInstance();
    
    size_t addPublisher(const std::string& topic_name);
    size_t addSubscriber(const std::string& topic_name, std::shared_ptr<AsyncSubscriberBase> subscriber);
    size_t addRequest(const std::string& topic_name);
    size_t addResponse(const std::string& topic_name);

    void sendMessage(size_t topic_id, std::shared_ptr<MessageBase> msg) noexcept;
    void sendRequest(size_t topic_id, std::shared_ptr<MessageBase> msg) noexcept;
private:
    std::hash<std::string> hash_function_;
    std::unordered_set<size_t> publishers_;
    std::unordered_map<size_t, std::vector<std::shared_ptr<AsyncSubscriberBase>>> subscribers_;
    std::unordered_set<size_t> requests_;
    std::unordered_set<size_t> responses_;
};



class AsyncNode : public SingleThread {
public:
    AsyncNode();
protected:
    size_t addPublisher(const std::string& topic_name);
    size_t addSubscriber(const std::string& topic_name, std::shared_ptr<AsyncSubscriberBase> subscriber);
    size_t addRequest(const std::string& topic_name);
    size_t addResponse(const std::string& topic_name);

    void sendMessage(size_t topic_id, std::shared_ptr<MessageBase> msg) noexcept;
    void sendRequest(size_t topic_id, std::shared_ptr<MessageBase> msg) noexcept;
    
private:   
    std::shared_ptr<AsyncSystem> system_;  
};


template<typename MessageT>
class AsyncSubscriber : public AsyncSubscriberBase{
public: 
    using MessagePtrT = std::shared_ptr<MessageT>;
    using MsgHandlerT = std::function<void(const MessagePtrT& msg)>;

    AsyncSubscriber(AsyncNode* node, MsgHandlerT msg_handler) 
    : node_(node) 
    , msg_handler_(msg_handler) {
    }

    void writeMessage(const std::shared_ptr<MessageBase>& msg) override {
        auto task_body = [this, msg]() { 
            msg_handler_(std::move(std::static_pointer_cast<MessageT>(msg)));
        };

        node_->addTask(task_body);
    }
private:
    AsyncNode* node_;
    MsgHandlerT msg_handler_;
};

template<typename RequestMsgT, typename ResponseMsgT>
class AsyncSubscriber : public AsyncRequestResponseBase {
public:
    using MessagePtrT = std::shared_ptr<MessageT>;

private:

};