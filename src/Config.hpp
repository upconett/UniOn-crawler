#include <string>
#include "DotEnvParser.hpp"


class Config {

public:
    const std::string phone_number;
    const std::string password;
    const std::string api_hash;
    const unsigned long api_id;

    static Config read(const std::string& env_path) {
        DotEnvParser p(env_path);
        return Config(
            p.get_str("PHONE_NUMBER"),
            p.get_str("PASSWORD"),
            p.get_str("API_HASH"),
            p.get_ulong("API_ID")
        );
    }

    /// @brief checks if config is valid, prints errors to cout
    /// @param env_path 
    /// @return true if valid, false otherwise
    static bool valid(const std::string& env_path) {
        try { 
            read(env_path);
            return true;
        } catch (io_fail) {
            std::cout << "No .env file!" << std::endl;
        } catch (no_value& e) {
            std::cout << ".env file misses " << e.what() << std::endl;
        }
        return false;
    }

protected:
    Config(
        const std::string phone_number,
        const std::string password,
        const std::string api_hash,
        const unsigned long api_id
    ) 
    : phone_number(phone_number),
      password(password), 
      api_hash(api_hash),
      api_id(api_id)
    { }

};
