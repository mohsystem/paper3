#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

class MemoryResource {
public:
    explicit MemoryResource(std::size_t size) {
        const std::size_t MAX_CAPACITY = 10'000'000u;
        if (size == 0 || size > MAX_CAPACITY) {
            throw std::invalid_argument("Invalid size");
        }
        data_.assign(size, 0u);
        closed_ = false;
    }

    std::size_t capacity() const {
        return closed_ ? 0u : data_.size();
    }

    bool isClosed() const {
        return closed_;
    }

    int write(std::size_t offset, const std::string& input) {
        if (closed_) return -1;
        if (offset >= data_.size()) return -1;
        std::size_t writable = std::min<std::size_t>(input.size(), data_.size() - offset);
        if (writable == 0) return -1;
        std::copy_n(reinterpret_cast<const unsigned char*>(input.data()), writable, data_.begin() + static_cast<std::ptrdiff_t>(offset));
        return static_cast<int>(writable);
    }

    std::string read(std::size_t offset, std::size_t length) const {
        if (closed_) return std::string();
        if (offset >= data_.size()) return std::string();
        std::size_t readable = std::min<std::size_t>(length, data_.size() - offset);
        if (readable == 0) return std::string();
        return std::string(reinterpret_cast<const char*>(&data_[offset]), readable);
    }

    void close() {
        if (!closed_) {
            std::fill(data_.begin(), data_.end(), 0u);
            std::vector<unsigned char>().swap(data_); // free memory
            closed_ = true;
        }
    }

    ~MemoryResource() {
        // Ensure zeroization even if not explicitly closed
        if (!closed_) {
            std::fill(data_.begin(), data_.end(), 0u);
        }
    }

private:
    std::vector<unsigned char> data_;
    bool closed_ = false;
};

int main() {
    try {
        // Test 1: Basic allocation, write, read
        {
            MemoryResource mr(16);
            int w = mr.write(0, "Hello");
            std::string r = mr.read(0, 5);
            std::cout << "T1: written=" << w << " read=\"" << r << "\" cap=" << mr.capacity() << "\n";
        }

        // Test 2: Partial write near end
        {
            MemoryResource mr(16);
            int w = mr.write(13, "World!"); // expect 3
            std::string r = mr.read(0, 16);
            std::cout << "T2: written=" << w << " read=\"" << r << "\"\n";
        }

        // Test 3: Write larger than capacity, ensure clamp
        {
            MemoryResource mr(8);
            int w = mr.write(0, "123456789"); // expect 8
            std::string r = mr.read(0, 8);
            std::cout << "T3: written=" << w << " read=\"" << r << "\"\n";
        }

        // Test 4: Use after close should fail
        {
            MemoryResource mr(10);
            mr.close();
            int w = mr.write(0, "X"); // expect -1
            std::string r = mr.read(0, 1); // expect empty
            std::cout << "T4: written=" << w << " readLen=" << r.size() << " isClosed=" << std::boolalpha << mr.isClosed() << "\n";
        }

        // Test 5: Invalid offset (too large)
        {
            MemoryResource mr(4);
            int w = mr.write(100, "AB"); // expect -1
            std::cout << "T5: written=" << w << " cap=" << mr.capacity() << "\n";
        }
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}