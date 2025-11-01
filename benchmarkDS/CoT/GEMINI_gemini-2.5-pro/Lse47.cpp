#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <filesystem>
#include <fstream>
#include <regex>

// A simple router to map URL patterns to handler functions
class SimpleRouter {
public:
    using Handler = std::function<std::string(const std::string&)>;

    // Simulates a decorator by registering a handler for a URL pattern
    void add_route(const std::string& pattern, Handler handler) {
        routes_[std::regex(pattern)] = handler;
    }

    // Dispatches a request to the appropriate handler
    std::string dispatch(const std::string& url) const {
        for (const auto& pair : routes_) {
            std::smatch match;
            if (std::regex_match(url, match, pair.first)) {
                if (match.size() > 1) {
                    return pair.second(match[1].str()); // Pass the captured filename
                }
            }
        }
        return "HTTP 404: Route not found.";
    }

private:
    std::map<std::regex, Handler> routes_;
};

class Lse47 {
private:
    const std::filesystem::path image_dir_ = "images";

public:
    Lse47() {
        // Ensure the image directory exists for the application
        std::filesystem::create_directory(image_dir_);
    }

    // The handler function for serving images
    std::string serveImage(const std::string& filename) {
        // --- Security Check: Path Traversal ---
        // Basic check for directory traversal characters.
        if (filename.find("..") != std::string::npos || filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
            return "HTTP 400: Invalid filename.";
        }

        try {
            // Get the canonical (absolute, resolved) path of the image directory
            auto base_path = std::filesystem::canonical(image_dir_);
            auto requested_path = base_path / filename;
            
            // Canonicalize the requested path to resolve any symlinks etc.
            auto canonical_req_path = std::filesystem::canonical(requested_path);

            // Final crucial check: ensure the requested file is inside the base directory
            std::string path_str = canonical_req_path.string();
            std::string base_str = base_path.string();
            if (path_str.rfind(base_str, 0) != 0) {
                 return "HTTP 403: Forbidden - Access outside of image directory is not allowed.";
            }

            if (std::filesystem::is_regular_file(canonical_req_path)) {
                std::ifstream file(canonical_req_path);
                if (file) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    return "HTTP 200: Serving content of " + filename + ": " + buffer.str();
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
             // This catch block handles cases where the file doesn't exist, preventing canonical() from throwing.
            return "HTTP 404: File not found or filesystem error.";
        }

        return "HTTP 404: File not found.";
    }
};

int main() {
    // 1. Setup: Create app instance which creates the directory
    Lse47 app;
    SimpleRouter router;

    // "Decorate" the serveImage method by adding it to the router
    router.add_route(
        "/images/([a-zA-Z0-9_.-]+)",
        [&app](const std::string& filename) {
            return app.serveImage(filename);
        }
    );
    
    // Create dummy files
    std::ofstream( "images/cat.jpg" ) << "This is a cat image";
    std::ofstream( "images/dog_1.png" ) << "This is a dog image";

    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Valid file request
    std::cout << "Test 1 (Valid): /images/cat.jpg" << std::endl;
    std::cout << "Response: " << router.dispatch("/images/cat.jpg") << std::endl << std::endl;

    // Test Case 2: File does not exist
    std::cout << "Test 2 (Not Found): /images/horse.gif" << std::endl;
    std::cout << "Response: " << router.dispatch("/images/horse.gif") << std::endl << std::endl;
    
    // Test Case 3: Path traversal attempt
    std::cout << "Test 3 (Path Traversal): /images/../secret.txt" << std::endl;
    std::cout << "Response: " << router.dispatch("/images/../secret.txt") << std::endl << std::endl; // Fails regex
    
    // Test Case 4: Another path traversal attempt (different format)
    std::cout << "Test 4 (Path Traversal 2): /images/..%2fsecret.txt" << std::endl;
    std::cout << "Response: " << router.dispatch("/images/..%2fsecret.txt") << std::endl << std::endl; // Fails regex

    // Test Case 5: Invalid route
    std::cout << "Test 5 (Invalid Route): /videos/movie.mp4" << std::endl;
    std::cout << "Response: " << router.dispatch("/videos/movie.mp4") << std::endl << std::endl;

    // Cleanup
    std::filesystem::remove_all("images");

    return 0;
}