#include <map>
#include <functional>
#include <mutex>

#include "td/telegram/td_api.h"


namespace td_api = td::td_api;


/// Singleton class, for managing request handlers  
/// ### thread safe
class HandlersManager {

    using Object = td_api::object_ptr<td_api::Object>;
    using HandlerFunc = std::function<void(Object)>;
    using ulong = unsigned long;

protected:
    HandlersManager() = default;

public:
    static HandlersManager& get_instance() {
        static HandlersManager instance;
        return instance;
    }

private:
    std::map<ulong, HandlerFunc> handlers;
    std::mutex mutex;

public:
    void add_handler(ulong id, HandlerFunc&& handler) {
        std::lock_guard lock(mutex);
        handlers.emplace(id, std::move(handler));
    }

    HandlerFunc* get_handler(ulong id) {
        std::lock_guard lock(mutex);
        auto found = handlers.find(id);
        if (found == handlers.end())
            return nullptr;
        else return &found->second;
    }

    void remove_handler(ulong id) {
        std::lock_guard lock(mutex);
        handlers.erase(id);
    }

    void execute_handler(ulong id, Object&& object) {
        HandlerFunc* handler = get_handler(id);
        try {
            if (handler != nullptr) {
                (*handler)(std::move(object));
                remove_handler(id);
            }
        } catch (std::bad_alloc e) {
            std::cout << "Caught bad_alloc in HandlersManager" << std::endl;
        }
    }

};
