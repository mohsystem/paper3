// Chain-of-Through Step 1: Problem understanding
// - Generate three random floats using rand() and convert to strings with sprintf-like formatting.
// - Store in variables: string, string2, string3. Function accepts parameters and returns output.

// Chain-of-Through Step 2: Security requirements
// - Use snprintf instead of sprintf to avoid buffer overflows.
// - Local seeding to avoid shared global state issues across calls in multithreaded contexts.

// Chain-of-Through Step 3: Secure coding generation
// - Deterministically seed std::rand with provided seed.
// - Format with precision using "%.*f".

// Chain-of-Through Step 4: Code review
// - Ensure buffers are sufficiently sized and snprintf return checked if desired.
// - Avoid 'using namespace std' to prevent name collisions.

// Chain-of-Through Step 5: Secure code output
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>

std::vector<std::string> generateRandomFloatStrings(unsigned int seed, int precision) {
    std::srand(seed);

    // Variables named as requested
    char string[64];
    char string2[64];
    char string3[64];

    double f1 = std::rand() / (static_cast<double>(RAND_MAX) + 1.0);
    double f2 = std::rand() / (static_cast<double>(RAND_MAX) + 1.0);
    double f3 = std::rand() / (static_cast<double>(RAND_MAX) + 1.0);

    int prec = precision < 0 ? 0 : precision;
    std::snprintf(string,  sizeof(string),  "%.*f", prec, f1);
    std::snprintf(string2, sizeof(string2), "%.*f", prec, f2);
    std::snprintf(string3, sizeof(string3), "%.*f", prec, f3);

    return {std::string(string), std::string(string2), std::string(string3)};
}

int main() {
    // 5 test cases
    std::vector<std::pair<unsigned int,int>> tests = {
        {0u, 6}, {1u, 4}, {42u, 2}, {123456789u, 8}, {5555u, 6}
    };

    for (const auto& t : tests) {
        auto v = generateRandomFloatStrings(t.first, t.second);
        std::cout << "seed=" << t.first << ", precision=" << t.second
                  << " -> [" << v[0] << ", " << v[1] << ", " << v[2] << "]\n";
    }
    return 0;
}