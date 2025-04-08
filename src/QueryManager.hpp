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

#include "HandlersManager.hpp"


class QueryManager {

    using Object = td_api::object_ptr<td_api::Object>;
    using Function = td_api::object_ptr<td_api::Function>;

private:
    HandlersManager& handler_manager;
    td::Client& client;

    std::mutex mutex;

    std::uint64_t current_query_id;

public:
    QueryManager(HandlersManager& handlers_manager, td::Client& client)
    : handler_manager(handlers_manager), client(client)
    { }

    void send(Function&& func, std::function<void(Object)> handler) {
        std::lock_guard lock(mutex);

        current_query_id++;
        if (handler != nullptr)
            handler_manager.add_handler(current_query_id, std::move(handler));

        client.send({current_query_id, std::move(func)});
    }

};
