#include <iostream>
#include <future>
#include <thread>

#include "td/telegram/Client.h"
#include "td/telegram/td_api.hpp"
#include "td/telegram/td_api.h"
#include "td/telegram/Log.h"

#include "util.cpp"


namespace td_api = td::td_api;


void handle_updateAuthorizationState(td::Client& client,td_api::updateAuthorizationState& update) {
        std::cout << "Received updateAuthorizationState: ";
        td_api::downcast_call(*update.authorization_state_, overloaded(
            [&client](td_api::authorizationStateClosed& state){ std::cout << "Closed"; },
            [&client](td_api::authorizationStateClosing& state){ std::cout << "Closing"; },
            [&client](td_api::authorizationStateWaitPhoneNumber& state){
                std::cout << "WaitPhoneNumber";
                auto request = td_api::make_object<td_api::setAuthenticationPhoneNumber>();
                request->phone_number_ = "";
                request->settings_ = td_api::make_object<td_api::phoneNumberAuthenticationSettings>();
                request->settings_->allow_flash_call_ = false;
                request->settings_->is_current_phone_number_ = true;
                client->send({4, std::move(request)});
            },
            [&client](td_api::authorizationStateWaitTdlibParameters& state){ 
                std::cout << "WaitTdLibParameters"; 
                auto request = td_api::make_object<td_api::setTdlibParameters>();
                request->api_id_ = 0;
                request->api_hash_ = "";
                request->database_directory_ = "tdlib";
                request->use_message_database_ = true;
                request->use_secret_chats_ = true;
                request->device_model_ = "Desktop";
                request->system_language_code_ = "en";
                request->application_version_ = "0.1";
                send_query(std::move(request), [&client](Object&& obj){
                    auto& ok = static_cast<td_api::ok &>(*obj);
                    std::cout << "OK\n";
                });
            },
            [&client](td_api::authorizationStateWaitCode& state) {
                std::cout << "WaitCode (enter please): ";
                auto request = td_api::make_object<td_api::checkAuthenticationCode>();
                std::cin >> request->code_;
                send_query(std::move(request), {});
            },
            [&client](td_api::authorizationStateWaitPassword& state){
                std::cout << "WaitPassword (enter please): ";
                auto request = td_api::make_object<td_api::checkAuthenticationPassword>();
                std::cin >> request->password_;
                send_query(std::move(request), {});
            },
            [&client](td_api::authorizationStateReady& state){ 
                std::cout << "Ready"; 
                get_me();
                // std::cout << " sending there: " << send_message->chat_id_ << "\n";
                // send_query(std::move(send_message), [&client](Object&& obj){
                //     if (isError(obj)) {
                //         auto& err = (td_api::error &) *obj;
                //         std::cout << err.message_ << '\n';
                //         std::cout << err.code_ << '\n';
                //     } else {
                //         std::cout << "Sent succesfully\n";
                //     }
                // });
            },
            [&client](td_api::Object& state){ std::cout << "Unknown [" << state.get_id() << ']'; }
        ));
        std::cout << "\n";
    }