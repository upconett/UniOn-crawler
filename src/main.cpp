#include <iostream>
#include "td_future.cpp"


int main() {
    if (!Config::valid(".env")) { exit(1); }

    Config config = Config::read(".env");

    App app(config, 1);
    app.run();

    return 0;
}
