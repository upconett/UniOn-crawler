#include <iostream>

#include <curl/curl.h>


int main() {
    CURL* curl;

    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {

        curl_easy_setopt(curl, CURLOPT_URL, "https://httpbin.org/ip");

        auto res = curl_easy_perform(curl);

        if (res != CURLE_OK)
            std::cout << curl_easy_strerror(res) << std::endl;

        // char* ct = NULL;
        // res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);

        // std::cout << "GETINFO " << res << std::endl;

        // if (ct) std::cout << ct << std::endl;

        
        curl_easy_cleanup(curl);
    }

    return 0;
}