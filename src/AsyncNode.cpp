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

size_t AsyncSystem::addSubscriber(const std::string& topic_name, std::shared_ptr<AsyncSubscriberBase> subscriber) {
    const size_t topic_id = hash_function_(topic_name);

    ASSERT(subscribers_.end() == subscribers_.find(topic_id), 
        "Non-unique hash topic ID value. Please choose another topic name for " + topic_name);

    subscribers_[topic_id].push_back(subscriber);

    return topic_id;
}

size_t AsyncSystem::addRequest(const std::string& topic_name) {
    const size_t topic_id = hash_function_(topic_name);

    ASSERT(requests_.end() == requests_.find(topic_id), 
        "Non-unique hash topic ID value. Please choose another topic name for " + topic_name);

    requests_.insert(topic_id);

    return topic_id;
}

size_t AsyncSystem::addResponse(const std::string& topic_name) {
    const size_t topic_id = hash_function_(topic_name);

    ASSERT(responses_.end() == responses_.find(topic_id), 
        "Non-unique hash topic ID value. Please choose another topic name for " + topic_name);

    responses_.insert(topic_id);

    return topic_id;
}

void AsyncSystem::sendMessage(size_t topic_id, std::shared_ptr<MessageBase> msg) noexcept {
    auto itr = subscribers_.find(topic_id);  
    if (subscribers_.end() != itr) {
        for(auto& subscriber : itr->second) {
            subscriber->writeMessage(msg);
        }
    } else {
        //TODO: log invalid topic_id 
    }
};

void AsyncSystem::sendRequest(size_t topic_id, std::shared_ptr<MessageBase> msg) noexcept {
    auto itr = responses_.find(topic_id);
    if (responses_.end() != itr) {
        //itr->second
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

size_t AsyncNode::addSubscriber(const std::string& topic_name, std::shared_ptr<AsyncSubscriberBase> subscriber) {
    return system_->addSubscriber(topic_name, subscriber);
}

size_t AsyncNode::addRequest(const std::string& topic_name) {
    return system_->addRequest(topic_name);
}

size_t AsyncNode::addResponse(const std::string& topic_name) {
    return system_->addResponse(topic_name);
}

void AsyncNode::sendMessage(size_t topic_id, std::shared_ptr<MessageBase> msg) noexcept {
    system_->sendMessage(topic_id, std::move(msg));
};

void AsyncNode::sendRequest(size_t topic_id, std::shared_ptr<MessageBase> msg) noexcept {
    system_->sendRequest(topic_id, std::move(msg));
};

