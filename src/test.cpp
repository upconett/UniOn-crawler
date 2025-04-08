#include <iostream>
#include <memory>
#include <map>
#include <functional>

#include "td/telegram/Client.h"
#include "td/telegram/td_api.h"
#include "td/telegram/Log.h"

#include "util.cpp"


namespace td_api = td::td_api;


void handle_updateOption(td_api::updateOption&);
void handle_updateAuthorizationState(td_api::updateAuthorizationState&);
void process_response(td::Client::Response&&);


class Test {

    using Object = td_api::object_ptr<td_api::Object>;
    using Function = td_api::object_ptr<td_api::Function>;

public:
    Test() 
    : client(std::make_shared<td::Client>()), 
      log(std::make_shared<td::Log>())
    {
        log->set_verbosity_level(2);
        process_response(client->receive(10));
    }

    void loop() {
        while (true) {
            process_response(client->receive(2));
        }
    }
private:
    std::shared_ptr<td::Client> client;
    std::shared_ptr<td::Log> log;

    std::uint64_t current_query_id = 0;

    std::map<std::uint64_t, std::function<void(Object)>> handlers;


    bool isError(Object& obj) { return obj->get_id() == td_api::error::ID; }


    void send_query(Function&& func, std::function<void(Object)> handler) {
        current_query_id++;
        if (handler != nullptr)
            handlers.emplace(current_query_id, std::move(handler));
        client->send({current_query_id, std::move(func)});
    }

    void process_response(td::Client::Response&& response) {
        if (response.object == nullptr) {
            std::cout << "Retrying...\n";
        } else {
            if (response.id != 0) {
                std::cout << "Got update: " << response.id << '\n';
                if (handlers.find(response.id) != handlers.end()) {
                    handlers[response.id](std::move(response.object));
                    auto r = handlers.erase(response.id);
                }
            } else {
                td_api::downcast_call(*response.object, overloaded(
                    [this](td_api::updateAuthorizationState& update){ handle_updateAuthorizationState(update); },
                    [this](td_api::updateOption& update){ handle_updateOption(update); },
                    [this](td_api::error& update){ std::cout << "ERROR: [" << update.code_ << "] " << update.message_ << "\n"; },
                    [this](td_api::Object& obj) { 
                        // std::cout << td_api::to_string(obj) << "\n";
                    }
                ));
            }
        }
    }


    void handle_updateOption(td_api::updateOption& update){
        return;
        std::cout << "Received updateOption:\n";
        std::cout << "name_: " << update.name_;
        if (update.name_ == "version") {
            auto& value = static_cast<td_api::optionValueString &>(*update.value_);
            std::cout << " value_: " << value.value_;
        } else if (update.name_ == "commit_hash") {
            auto& value = static_cast<td_api::optionValueString &>(*update.value_);
            std::cout << " value_: " << value.value_;
        }
        std::cout << std::endl;
    }

    void log_error(Object& obj) {
        auto& err = (td_api::error&) *obj;
        std::cout << "Error: [" << err.code_ << "] " << err.message_ << "\n";
    }

    void get_chat(td_api::int53 chat_id) {
        send_query(td_api::make_object<td_api::getChat>(chat_id), {});
    }

    void send_text_message(td_api::int53 chat_id, std::string text) {
        get_chat(chat_id);
        auto send_message = td_api::make_object<td_api::sendMessage>();
        send_message->chat_id_ = chat_id;
        auto message_content = td_api::make_object<td_api::inputMessageText>();
        message_content->text_ = td_api::make_object<td_api::formattedText>();
        message_content->text_->text_ = std::move(text);
        send_message->input_message_content_ = std::move(message_content);
        send_query(std::move(send_message), [this](Object&& obj){ if (isError(obj)) log_error(obj); });
    }

    void get_me() {
        auto request = td_api::make_object<td_api::getMe>();
        send_query(std::move(request), [this](Object&& obj){
            if (isError(obj)) log_error(obj);
        });
    }

    void handle_updateAuthorizationState(td_api::updateAuthorizationState& update) {
        std::cout << "Received updateAuthorizationState: ";
        td_api::downcast_call(*update.authorization_state_, overloaded(
            [this](td_api::authorizationStateClosed& state){ std::cout << "Closed"; },
            [this](td_api::authorizationStateClosing& state){ std::cout << "Closing"; },
            [this](td_api::authorizationStateWaitPhoneNumber& state){
                std::cout << "WaitPhoneNumber";
                auto request = td_api::make_object<td_api::setAuthenticationPhoneNumber>();
                request->phone_number_ = "";
                request->settings_ = td_api::make_object<td_api::phoneNumberAuthenticationSettings>();
                request->settings_->allow_flash_call_ = false;
                request->settings_->is_current_phone_number_ = true;
                client->send({4, std::move(request)});
            },
            [this](td_api::authorizationStateWaitTdlibParameters& state){ 
                std::cout << "WaitTdLibParameters"; 
                auto request = td_api::make_object<td_api::setTdlibParameters>();
                request->api_id_ = 2;
                request->api_hash_ = "";
                request->database_directory_ = "tdlib";
                request->use_message_database_ = true;
                request->use_secret_chats_ = true;
                request->device_model_ = "Desktop";
                request->system_language_code_ = "en";
                request->application_version_ = "0.1";
                send_query(std::move(request), [this](Object&& obj){
                    auto& ok = static_cast<td_api::ok &>(*obj);
                    std::cout << "OK\n";
                });
            },
            [this](td_api::authorizationStateWaitCode& state) {
                std::cout << "WaitCode (enter please): ";
                auto request = td_api::make_object<td_api::checkAuthenticationCode>();
                std::cin >> request->code_;
                send_query(std::move(request), {});
            },
            [this](td_api::authorizationStateWaitPassword& state){
                std::cout << "WaitPassword (enter please): ";
                auto request = td_api::make_object<td_api::checkAuthenticationPassword>();
                std::cin >> request->password_;
                send_query(std::move(request), {});
            },
            [this](td_api::authorizationStateReady& state){ 
                std::cout << "Ready"; 
                get_me();
                // std::cout << " sending there: " << send_message->chat_id_ << "\n";
                // send_query(std::move(send_message), [this](Object&& obj){
                //     if (isError(obj)) {
                //         auto& err = (td_api::error &) *obj;
                //         std::cout << err.message_ << '\n';
                //         std::cout << err.code_ << '\n';
                //     } else {
                //         std::cout << "Sent succesfully\n";
                //     }
                // });
            },
            [this](td_api::Object& state){ std::cout << "Unknown [" << state.get_id() << ']'; }
        ));
        std::cout << "\n";
    }

};


int main() {
    Test t;
    t.loop();
    return 0;
}