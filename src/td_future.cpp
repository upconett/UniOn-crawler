#include <iostream>
#include <future>
#include <thread>
#include <functional>
#include <map>
#include <list>

#include "td/telegram/Client.h"
#include "td/telegram/td_api.hpp"
#include "td/telegram/td_api.h"
#include "td/telegram/Log.h"

#include "util.cpp"
#include "ThreadPool.hpp"
#include "QueryManager.hpp"
#include "Config.hpp"


class request_exception : public std::exception {};


class App {

    td::Client client;

    HandlersManager& handlers_manager;
    QueryManager query_manager;

    Config config;

    ThreadPool thread_pool;

    void handle_updateAuthorizationState(td_api::updateAuthorizationState& update) {
        std::cout << "Received updateAuthorizationState: ";
        td_api::downcast_call(*update.authorization_state_, overloaded(
            [this](td_api::authorizationStateClosed& state){ std::cout << "Closed"; },
            [this](td_api::authorizationStateClosing& state){ std::cout << "Closing"; },
            [this](td_api::authorizationStateWaitPhoneNumber& state){
                std::cout << "WaitPhoneNumber";
                auto request = td_api::make_object<td_api::setAuthenticationPhoneNumber>();
                request->phone_number_ = config.phone_number;
                request->settings_ = td_api::make_object<td_api::phoneNumberAuthenticationSettings>();
                request->settings_->allow_flash_call_ = false;
                request->settings_->is_current_phone_number_ = true;
                query_manager.send(std::move(request), [](Object&& obj){
                    std::cout << td_api::to_string(obj) << std::endl;
                });
            },
            [this](td_api::authorizationStateWaitTdlibParameters& state){ 
                std::cout << "WaitTdLibParameters"; 
                auto request = td_api::make_object<td_api::setTdlibParameters>();
                request->api_id_ = config.api_id;
                request->api_hash_ = config.api_hash; 
                request->database_directory_ = "tdlib";
                request->use_message_database_ = true;
                request->use_secret_chats_ = true;
                request->device_model_ = "Desktop";
                request->system_language_code_ = "en";
                request->application_version_ = "0.1";
                query_manager.send(std::move(request), {});
            },
            [this](td_api::authorizationStateWaitCode& state) {
                std::cout << "WaitCode (enter please): ";
                auto request = td_api::make_object<td_api::checkAuthenticationCode>();
                std::cin >> request->code_;
                query_manager.send(std::move(request), {});
            },
            [this](td_api::authorizationStateWaitPassword& state){
                // std::cout << "WaitPassword (enter please): ";
                // std::cin >> request->password_;
                auto request = td_api::make_object<td_api::checkAuthenticationPassword>();
                request->password_ = config.password;
                query_manager.send(std::move(request), {});
            },
            [this](td_api::authorizationStateReady& state){ 
                std::cout << "Ready\n"; 
            },
            [this](td_api::Object& state){ std::cout << "Unknown [" << state.get_id() << ']'; }
        ));
        std::cout << "\n";
    }

    std::future<std::string> get_chat_name(td_api::int53 chat_id) {
        auto promise = std::make_shared<std::promise<std::string>>();
        auto future = promise->get_future();

        query_manager.send(
            td_api::make_object<td_api::getChat>(chat_id),
            [this, promise](Object&& obj) {
                if (isError(obj)) {
                    log_error(obj);
                    promise->set_exception(std::make_exception_ptr(request_exception()));
                } else {
                    td_api::chat& chat = static_cast<td_api::chat&>(*obj);
                    promise->set_value(chat.title_);
                }
            }
        );

        return future;
    }

    std::future<std::string> get_message_link(td_api::int53 chat_id, td_api::int53 message_id) {
        auto promise = std::make_shared<std::promise<std::string>>();
        auto future = promise->get_future();

        query_manager.send(
            td_api::make_object<td_api::getMessageLink>(chat_id, message_id, 0, 0, 0),
            [this, promise](Object&& obj) {
                if (isError(obj)) {
                    log_error(obj);
                    promise->set_exception(std::make_exception_ptr(request_exception()));
                } else {
                    td_api::messageLink& link = static_cast<td_api::messageLink&>(*obj);
                    promise->set_value(link.link_);
                }
            }
        );

        return future;
    }


    void handle_updateNewMessage(td_api::updateNewMessage& update) {
        std::cout << "New message: " << update.message_->id_ << 
            ", chat_id: " << update.message_->chat_id_ << "\n" << std::flush;
        std::cout << "Trying to get chat name...\n" << std::flush;


        std::future<std::string> chat_name_future = get_chat_name(update.message_->chat_id_);
        std::future<std::string> link_future = get_message_link(update.message_->chat_id_, update.message_->id_);

        std::string chat_name = chat_name_future.get();
        std::string message_link = link_future.get();

        std::cout << "That was a message from: " << chat_name <<" link: " << message_link << "\n";
    }

    void processResponse(td::Client::Response&& response) {
        if (response.object != nullptr) {
            if (response.id != 0) {
                std::cout << "Got update: " << response.id << '\n';
                handlers_manager.execute_handler(response.id, std::move(response.object));
            } else {
                td_api::downcast_call(*response.object, overloaded(
                    [this](td_api::updateAuthorizationState& update){ handle_updateAuthorizationState(update); },
                    [this](td_api::updateNewMessage& update) {
                        thread_pool.add_task(
                            [this](td_api::updateNewMessage&& update){ 
                                handle_updateNewMessage(update);
                            },
                            std::move(update)
                        );
                    },
                    [this](td_api::error& update){ std::cout << "ERROR: [" << update.code_ << "] " << update.message_ << "\n"; },
                    [this](td_api::Object& obj) { }
                ));
            }
        }
    };

public:
    App(
        Config config,
        int log_verbosity_level
    )
    : thread_pool(10),
      handlers_manager(HandlersManager::get_instance()),
      query_manager(QueryManager(handlers_manager, client)),
      config(config)
    {
        td::Log().set_verbosity_level(log_verbosity_level);
    }

    void run() {
        std::cout << "Running App..." << std::endl;
        try {
            while (true) {
                processResponse(
                    std::move(client.receive(1))
                );
            }
        } catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Exception caught: " << e.what() << std::endl;
        }
    }
};
