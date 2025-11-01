#include <iostream>
#include <cstring> // For memmove
#include <vector>   // For safe buffer management in main
#include <iomanip>  // For std::hex
#include <string>   // For std::string
#include <cctype>   // For isprint

/**
 * @brief Copies a number of bytes from one memory location to another.
 *
 * This function is a wrapper around memmove. It is the caller's responsibility
 * to ensure that the destination buffer is large enough to hold 'num_bytes'
 * and that the source buffer contains at least 'num_bytes'.
 * This implementation adds checks for NULL pointers.
 * memmove is used instead of memcpy to correctly handle overlapping memory regions.
 *
 * @param dest Pointer to the destination memory block.
 * @param src Pointer to the source memory block.
 * @param num_bytes The number of bytes to copy.
 */
void copyBytes(void* dest, const void* src, size_t num_bytes) {
    // Rule #4: Validate inputs. Check for null pointers.
    if (dest == nullptr || src == nullptr) {
        // Silently return on null input to prevent crashes.
        // A more robust implementation might log an error.
        return;
    }

    if (num_bytes == 0) {
        return; // Nothing to copy.
    }

    // Rules #1, #2, #5: Use a safer function (memmove) for the copy.
    // memmove correctly handles overlapping memory regions, which is safer
    // than memcpy. The core responsibility for bounds checking for raw
    // pointers lies with the caller. This function cannot know the allocated
    // sizes of `dest` and `src`.
    memmove(dest, src, num_bytes);
}

void print_buffer(const std::string& name, const std::vector<char>& buffer) {
    std::cout << name << ": ";
    for (const auto& c : buffer) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(c) & 0xff) << " ";
    }
    std::cout << "(str: \"";
    for(const auto& c : buffer){
        if(c != '\0' && isprint(static_cast<unsigned char>(c))) std::cout << c; else std::cout << ".";
    }
    std::cout << "\")" << std::endl;
}

int main() {
    // Test Case 1: Normal successful copy
    std::cout << "--- Test Case 1: Normal Copy ---" << std::endl;
    std::vector<char> src1 = {'H', 'e', 'l', 'l', 'o', '\0'};
    std::vector<char> dest1(6, 0);
    copyBytes(dest1.data(), src1.data(), 6);
    print_buffer("Source     ", src1);
    print_buffer("Destination", dest1);
    std::cout << std::endl;

    // Test Case 2: Overlapping memory (dest starts within src)
    std::cout << "--- Test Case 2: Overlapping Copy (dest > src) ---" << std::endl;
    std::vector<char> buffer2 = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '\0'};
    print_buffer("Before", buffer2);
    copyBytes(buffer2.data() + 2, buffer2.data(), 5); // copy "12345" to buffer[2]
    print_buffer("After ", buffer2);
    std::cout << std::endl;

    // Test Case 3: Overlapping memory (src starts within dest)
    std::cout << "--- Test Case 3: Overlapping Copy (src > dest) ---" << std::endl;
    std::vector<char> buffer3 = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '\0'};
    print_buffer("Before", buffer3);
    copyBytes(buffer3.data(), buffer3.data() + 2, 5); // copy "34567" to buffer[0]
    print_buffer("After ", buffer3);
    std::cout << std::endl;

    // Test Case 4: Copy zero bytes
    std::cout << "--- Test Case 4: Zero Bytes Copy ---" << std::endl;
    std::vector<char> src4 = {'A', 'B', 'C', '\0'};
    std::vector<char> dest4 = {'X', 'Y', 'Z', '\0'};
    print_buffer("Dest Before", dest4);
    copyBytes(dest4.data(), src4.data(), 0);
    print_buffer("Dest After ", dest4);
    std::cout << "Result: Destination is unchanged" << std::endl;
    std::cout << std::endl;

    // Test Case 5: Null pointer input
    std::cout << "--- Test Case 5: Null Input ---" << std::endl;
    std::vector<char> src5 = {'A', 'B', 'C', '\0'};
    copyBytes(nullptr, src5.data(), 3);
    copyBytes(src5.data(), nullptr, 3);
    std::cout << "Result: Program did not crash (as expected)." << std::endl;
    print_buffer("Src (should be unchanged)", src5);
    std::cout << std::endl;

    return 0;
}