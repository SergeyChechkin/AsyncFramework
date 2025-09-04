#include "../include/AsyncFramework/AsyncNode.h"

static std::shared_ptr<AsyncSystem> system_;

std::shared_ptr<AsyncSystem> AsyncSystem::getInstance() {
    if UNLIKELY(!system_) {
        system_ = std::make_shared<AsyncSystem>();
    }

    return system_;
}

size_t AsyncSystem::addPublisher(const std::string& topic_name) {
    const size_t topic_id = hash_function_(topic_name);

    ASSERT(publishers_.end() == publishers_.find(topic_id), 
        "Non-unique hash topic ID value. Please choose another topic name for " + topic_name);

    publishers_.insert(topic_id);

    return topic_id;
}

size_t AsyncSystem::addSubscriber(const std::string& topic_name, std::shared_ptr<MessageReceiver> subscriber) {
    const size_t topic_id = hash_function_(topic_name);

    ASSERT(subscribers_.end() == subscribers_.find(topic_id), 
        "Non-unique hash topic ID value. Please choose another topic name for " + topic_name);

    subscribers_[topic_id].push_back(std::move(subscriber));

    return topic_id;
}

PairID AsyncSystem::addRequest(
        const std::string& request_topic_name, 
        const std::string& responder_name, 
        std::shared_ptr<ResponseReceiver> request_handler) 
{
    
    const PairID request_id = {hash_function_(request_topic_name), hash_function_(responder_name)}; 
            
    ASSERT(requesters_.end() == requesters_.find(request_id), 
        "AsyncSystem::addRequest(...): Non-unique hash request ID value. Please choose another topic name for " + responder_name);

    requesters_[request_id] = std::move(request_handler);
      
    return request_id;    
}

size_t AsyncSystem::addResponse(const std::string& topic_name, std::shared_ptr<RequestReceiver> request_handler) {
    const size_t topic_id = hash_function_(topic_name);

    ASSERT(responders_.end() == responders_.find(topic_id), 
        "Non-unique hash topic ID value. Please choose another topic name for " + topic_name);

    responders_[topic_id] = request_handler;

    return topic_id;
}

void AsyncSystem::sendMessage(size_t topic_id, MessageBasePtr msg) noexcept {
    auto itr = subscribers_.find(topic_id);  
    if (subscribers_.end() != itr) {
        for(auto& subscriber : itr->second) {
            subscriber->writeMessage(msg);
        }
    } else {
        //TODO: log invalid pub topic_id 
    }
};

void AsyncSystem::sendRequest(PairID request_id, MessageBasePtr request) noexcept {
    auto itr = responders_.find(request_id.first_);
    if (responders_.end() != itr) {
        itr->second->writeRequest(request_id, std::move(request));
    } else {
        //TODO: log invalid request topic_id
    }
}

void AsyncSystem::sendResponse(PairID request_id, MessageBasePtr request, MessageBasePtr responce) noexcept {
    auto itr = requesters_.find(request_id);
    if (requesters_.end() != itr) {
        itr->second->writeResponse(std::move(request), std::move(responce));
    } else {
        //TODO: log invalid request topic_id
    }
}


AsyncNode::AsyncNode() 
: SingleThread()
, system_(AsyncSystem::getInstance())  {
} 

size_t AsyncNode::addPublisher(const std::string& topic_name) {
    return system_->addPublisher(topic_name);
}

size_t AsyncNode::addSubscriber(const std::string& topic_name, std::shared_ptr<MessageReceiver> subscriber) {
    return system_->addSubscriber(topic_name, subscriber);
}

PairID AsyncNode::addRequest(
        const std::string& request_topic_name, 
        const std::string& responder_name, 
        std::shared_ptr<ResponseReceiver> request_handler) {
            return system_->addRequest(request_topic_name, responder_name, request_handler);
        }

size_t AsyncNode::addResponse(const std::string& topic_name, std::shared_ptr<RequestReceiver> request_handler) {
    return system_->addResponse(topic_name, request_handler);
}

void AsyncNode::sendMessage(size_t topic_id, std::shared_ptr<MessageBase> msg) noexcept {
    system_->sendMessage(topic_id, std::move(msg));
}

void AsyncNode::sendRequest(PairID request_id, MessageBasePtr request) noexcept {
    system_->sendRequest(request_id, request);
}

void AsyncNode::sendResponse(PairID request_id, MessageBasePtr request, MessageBasePtr responce) noexcept {
    system_->sendResponse(request_id, request, responce);
}


