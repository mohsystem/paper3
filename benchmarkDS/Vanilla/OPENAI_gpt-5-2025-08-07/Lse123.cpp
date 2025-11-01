#include <iostream>
#include <string>
#include <vector>
#include <any>

class Lse123 {
public:
    static std::string processArgs(const std::vector<std::any>& args) {
        if (args.size() != 1) {
            return std::string();
        }
        if (!args[0].has_value() || args[0].type() != typeid(std::string)) {
            return std::string();
        }
        std::string s = std::any_cast<std::string>(args[0]);

        int end = static_cast<int>(s.size()) - 1;
        for (int i = end; i >= 0; --i) {
            char c = s[i];
            if (c == ' ' || c == '\t') {
                end = i - 1;
            } else {
                break;
            }
        }
        s.resize(end + 1 > 0 ? end + 1 : 0);

        end = static_cast<int>(s.size()) - 1;
        for (int i = end; i >= 0; --i) {
            char c = s[i];
            if (c == '\n' || c == '\r') {
                end = i - 1;
            } else {
                break;
            }
        }
        s.resize(end + 1 > 0 ? end + 1 : 0);

        s.push_back('\0');
        return s;
    }
};

int main() {
    std::vector<std::any> t1 = { std::string("Hello World   \t  \n\n") };
    std::vector<std::any> t2 = { std::string("NoTrim") };
    std::vector<std::any> t3 = { std::string("a"), std::string("b") };
    std::vector<std::any> t4 = { 42 };
    std::vector<std::any> t5 = { std::string("abc \t \r\n\r\n") };

    std::string r1 = Lse123::processArgs(t1);
    std::string r2 = Lse123::processArgs(t2);
    std::string r3 = Lse123::processArgs(t3);
    std::string r4 = Lse123::processArgs(t4);
    std::string r5 = Lse123::processArgs(t5);

    auto out = [](const std::string& s, int idx){
        if (s.empty()) {
            std::cout << "Test " << idx << ": null\n";
        } else {
            std::cout << "Test " << idx << ": len=" << s.size() << ", value='";
            std::cout << s;
            std::cout << "'\n";
        }
    };

    out(r1, 1);
    out(r2, 2);
    out(r3, 3);
    out(r4, 4);
    out(r5, 5);

    return 0;
}