#pragma once

#include "threads/SingleThread.h"
#include "common/macros.h"

#include <memory> 
#include <unordered_map> 
#include <unordered_set> 
#include <functional>

struct PairID {
    size_t first_;
    size_t second_;

    friend bool operator == (const PairID& left, const PairID& right) {
        return left.first_ == right.first_ && left.second_ == right.second_; 
    } 
    
    size_t operator() (const PairID& val) const {
        return val.first_ ^ val.second_;
    }
};

// Abstract class for message
class MessageBase {
public:    
};

class MessageReceiver {
public:
    using MessageBasePtr =  std::shared_ptr<MessageBase>;
    virtual void writeMessage(const MessageBasePtr& msg) = 0;
};

class ResponseReceiver {
public:
    using MessageBasePtr = std::shared_ptr<MessageBase>;
    virtual void writeResponse(const MessageBasePtr& request, const MessageBasePtr& responce) = 0;
};

class RequestReceiver {
public:
    using MessageBasePtr = std::shared_ptr<MessageBase>;
    virtual void writeRequest(PairID request_id, const MessageBasePtr& request) = 0;
};



class AsyncSystem {
public:
    using MessageBasePtr = std::shared_ptr<MessageBase>;

    static std::shared_ptr<AsyncSystem> getInstance();
    
    size_t addPublisher(const std::string& topic_name);
    size_t addSubscriber(const std::string& topic_name, std::shared_ptr<MessageReceiver> subscriber);
    PairID addRequest(
        const std::string& request_topic_name, 
        const std::string& responder_name, 
        std::shared_ptr<ResponseReceiver> request_handler);
    size_t addResponse(const std::string& topic_name, std::shared_ptr<RequestReceiver> request_handler);

    void sendMessage(size_t topic_id, MessageBasePtr msg) noexcept;
    void sendRequest(PairID request_id, MessageBasePtr request) noexcept;
    void sendResponse(PairID request_id, MessageBasePtr request, MessageBasePtr responce) noexcept;
private:
    std::hash<std::string> hash_function_;
    std::unordered_set<size_t> publishers_;
    std::unordered_map<size_t, std::vector<std::shared_ptr<MessageReceiver>>> subscribers_;

    std::unordered_set<size_t> requests_;
    std::unordered_map<PairID, std::shared_ptr<ResponseReceiver>, PairID> requesters_;
    std::unordered_map<size_t, std::shared_ptr<RequestReceiver>> responders_;
};



class AsyncNode : public SingleThread {
public:
    using MessageBasePtr = std::shared_ptr<MessageBase>;

    AsyncNode();

    void sendMessage(size_t topic_id, std::shared_ptr<MessageBase> msg) noexcept;
    void sendRequest(PairID request_id, MessageBasePtr request) noexcept;
    void sendResponse(PairID request_id, MessageBasePtr request, MessageBasePtr responce) noexcept;
protected:
    size_t addPublisher(const std::string& topic_name);
    size_t addSubscriber(const std::string& topic_name, std::shared_ptr<MessageReceiver> subscriber);
    PairID addRequest(
        const std::string& request_topic_name, 
        const std::string& responder_name, 
        std::shared_ptr<ResponseReceiver> request_handler);
    size_t addResponse(const std::string& topic_name, std::shared_ptr<RequestReceiver> request_handler);
    
    
private:   
    std::shared_ptr<AsyncSystem> system_;  
};


template<typename MessageT>
class AsyncSubscriber : public MessageReceiver{
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
class AsyncResponseHandler : public ResponseReceiver {
public:
    using RequestMsgPtrT = std::shared_ptr<RequestMsgT>;
    using ResponseMsgPtrT = std::shared_ptr<ResponseMsgT>;
    using ResponseMsgHandlerT = std::function<void(const RequestMsgPtrT& request, const ResponseMsgPtrT& responce)>;

    AsyncResponseHandler(AsyncNode* node, ResponseMsgHandlerT response_handler) 
    : node_(node) 
    , response_handler_(response_handler) {
    }

    void writeResponse(const MessageBasePtr& request, const MessageBasePtr& responce) override {
        auto task_body = [this, request, responce]() { 
            response_handler_(
                std::move(std::static_pointer_cast<RequestMsgT>(request)), 
                std::move(std::static_pointer_cast<ResponseMsgT>(responce)));
        };

        node_->addTask(task_body);
    }

private:
    AsyncNode* node_;
    ResponseMsgHandlerT response_handler_;
};

template<typename RequestMsgT, typename ResponseMsgT>
class AsyncRequestHandler : public RequestReceiver {
public:
    using RequestMsgPtrT = std::shared_ptr<RequestMsgT>;
    using ResponseMsgPtrT = std::shared_ptr<ResponseMsgT>;
    using RequestHandlerT = std::function<ResponseMsgPtrT(const RequestMsgPtrT& request)>;


    AsyncRequestHandler(AsyncNode* node, RequestHandlerT request_handler) 
    : node_(node) 
    , request_handler_(request_handler) {
    }

    void writeRequest(PairID request_id, const MessageBasePtr& request) override {
        auto task_body = [this, request, request_id]() { 
            const auto response = request_handler_(std::static_pointer_cast<RequestMsgT>(request));
            node_->sendResponse(request_id, std::move(request), std::move(response));
        };

        node_->addTask(task_body);
        
    }

private:
    AsyncNode* node_;
    RequestHandlerT request_handler_;
};