#include <iostream>
#include <fstream>

class DENV {
    const std::string file_path;

public:
    DENV(const std::string file_path)
    : file_path(file_path)
    {
        std::ifstream in;
        in.open(file_path);

        std::cout << in.fail() << std::endl;

        std::string str;

        while (!in.eof()) {
            std::getline(in, str);
            std::cout << str << std::endl;
        }

        in.close();
    }
};


int main() {
    DENV d(".env");
}
