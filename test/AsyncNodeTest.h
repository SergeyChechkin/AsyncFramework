#pragma once

#include "AsyncFramework/AsyncNode.h"

#include <iostream>

// Just some data structure
class TestMessageWithData : public MessageBase {
public:    
    u_int64_t data_uint_;
    std::string data_string_;
};

class PublisherAsyncNode : AsyncNode {
public:
    PublisherAsyncNode() : AsyncNode() {
        publisher_id_ = AsyncNode::addPublisher("test_msg");
    }

    void sendMessages() {
        std::vector<std::shared_ptr<MessageBase>> msgs;
        for(int i = 0; i < 10; ++i) {
            auto msg = std::make_shared<TestMessageWithData>(); 
            msg->data_uint_ = i;
            msg->data_string_ = std::to_string(i);
            AsyncNode::sendMessage(publisher_id_, std::move(msg));
        }
    }

private:
    size_t publisher_id_; // TestMessageWithData
};

class SubscriberAsyncNode : public AsyncNode {
public:
    using MessagePtrT = std::shared_ptr<TestMessageWithData>;
    using MsgHandlerT = std::function<void(const MessagePtrT& msg)>;
    
    SubscriberAsyncNode() 
    {
        auto on_msg_body = [this](const MessagePtrT& msg) {this->msgHandler(msg);};
        addSubscriber("test_msg", std::make_shared<SubscriberT>(this, on_msg_body));  
    };
private:
    void msgHandler(const MessagePtrT& msg) {
        //std::cout << "SubscriberAsyncNode::msgHandler(...) " << std::endl; 
        std::cout << msg.use_count() << " " << msg->data_string_ << std::endl; 
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
};


class RequestAsyncNode : public AsyncNode {
public:
    using requestMsgT = TestMessageWithData; 
    using requestMsgPtrT = std::shared_ptr<TestMessageWithData>; 

    using responceMsgT = TestMessageWithData>; 
    using responceMsgPtrT = std::shared_ptr<responceMsgT>; 

    RequestAsyncNode(int id) : id_(id) {
        request_id_ = AsyncNode::addRequest("test_msg");
    }

    void sendRequests() {
        std::vector<std::shared_ptr<MessageBase>> msgs;
        for(int i = 0; i < 10; ++i) {
            auto msg = std::make_shared<TestMessageWithData>(); 
            msg->data_uint_ = i;
            msg->data_string_ = std::to_string(i);
            //AsyncNode::sendRequest(publisher_id_, std::move(msg));
        }
    }
private:
    int id_;
    size_t request_id_; // TestMessageWithData  
    
    void responceHandler(const requestMsgPtrT& request, const responceMsgPtrT& responce) {
        //std::cout << "SubscriberAsyncNode::msgHandler(...) " << std::endl; 
        //std::cout << msg.use_count() << " " << msg->data_string_ << std::endl; 
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
};

class ResponceAsyncNode : public AsyncNode {
public:
};


void AsyncNodeTest() {
    SubscriberAsyncNode sub_node;
    PublisherAsyncNode pub_node;

    pub_node.sendMessages();

    ResponceAsyncNode resp_node;
    RequestAsyncNode req_node_1(1);
    RequestAsyncNode req_node_2(2);

    req_node_1.sendRequests();
    req_node_2.sendRequests();

}



