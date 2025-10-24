#include <iostream>

class Task23 {
public:
    static long long findNb(long long m) {
        long long totalVolume = 0;
        long long n = 0;
        while (totalVolume < m) {
            n++;
            totalVolume += n * n * n;
        }
        if (totalVolume == m) {
            return n;
        } else {
            return -1;
        }
    }
};

int main() {
    std::cout << "findNb(1071225) -> " << Task23::findNb(1071225) << std::endl;
    std::cout << "findNb(91716553919377LL) -> " << Task23::findNb(91716553919377LL) << std::endl;
    std::cout << "findNb(4183059834009LL) -> " << Task23::findNb(4183059834009LL) << std::endl;
    std::cout << "findNb(1) -> " << Task23::findNb(1) << std::endl;
    std::cout << "findNb(135440716410000LL) -> " << Task23::findNb(135440716410000LL) << std::endl;
    return 0;
}