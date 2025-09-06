#pragma once

#include <async_framework/AsyncNode.h>
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
    using SubscriberT = AsyncSubscriber<TestMessageWithData>;    
    using MessagePtrT = std::shared_ptr<TestMessageWithData>;
    using MsgHandlerT = std::function<void(const MessagePtrT& msg)>;
    
    SubscriberAsyncNode() 
    {
        auto on_msg_body = [this](const MessagePtrT& msg) {this->msgHandler(std::move(msg));};
        addSubscriber("test_msg", std::make_shared<SubscriberT>(this, on_msg_body));  
    };
private:
    void msgHandler(const MessagePtrT& msg) {
        std::cout << msg.use_count() << " " << msg->data_string_ << std::endl; 
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
};


class RequestAsyncNode : public AsyncNode {
public:
    using RequestMsgT = TestMessageWithData; 
    using RequestMsgPtrT = std::shared_ptr<RequestMsgT>; 
    using ResponseMsgT = TestMessageWithData; 
    using ResponseMsgPtrT = std::shared_ptr<ResponseMsgT>; 
    using ResponceMsgHandlerT = AsyncResponseHandler<RequestMsgT, ResponseMsgT>;
    
    RequestAsyncNode(int id) : id_(id) {
        
        auto on_responce_body = [this](const ResponseMsgPtrT& request, const ResponseMsgPtrT& responce) {
            this->responceHandler(std::move(request), std::move(responce));
        };

        request_id_ = AsyncNode::addRequest(
            "test_msg", 
            "test_msg_" + std::to_string(id_), 
            std::make_shared<ResponceMsgHandlerT>(this, on_responce_body));
    }

    void sendRequests() {
        std::vector<std::shared_ptr<MessageBase>> msgs;
        for(int i = 0; i < 10; ++i) {
            auto msg = std::make_shared<TestMessageWithData>();
            msg->data_uint_ = i;
            msg->data_string_ = std::to_string(id_);
            AsyncNode::sendRequest(request_id_, std::move(msg));
            //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
private:
    int id_;
    PairID request_id_; 
    
    void responceHandler(const ResponseMsgPtrT& request, const ResponseMsgPtrT& responce) {
        std::cout << request->data_string_ << " " << request->data_uint_ << " " << responce->data_string_ << " " << responce->data_uint_ << std::endl;
    }
};

class ResponceAsyncNode : public AsyncNode {
public:
    using RequestMsgT = TestMessageWithData; 
    using RequestMsgPtrT = std::shared_ptr<RequestMsgT>; 
    using ResponseMsgT = TestMessageWithData; 
    using ResponseMsgPtrT = std::shared_ptr<ResponseMsgT>; 
    using RequestHandlerT = AsyncRequestHandler<RequestMsgT, ResponseMsgT>;

    ResponceAsyncNode() {
        auto on_request_body = [this](const RequestMsgPtrT& msg) {
            return this->requestHandler(std::move(msg));
        };

        AsyncNode::addResponse("test_msg", std::make_shared<RequestHandlerT>(this, on_request_body));
    }
private:
    ResponseMsgPtrT requestHandler(const ResponseMsgPtrT& request) {
        // Do some computation here ...
        const int result = request->data_uint_ * 2;
        // Compose response
        ResponseMsgPtrT response = std::make_shared<ResponseMsgT>();
        response->data_uint_ = result;
        response->data_string_ = request->data_string_;
        return response;
    }
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

    std::this_thread::sleep_for(std::chrono::seconds(1));
}



