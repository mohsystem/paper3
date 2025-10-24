/*
================================================================================
Instructions for C++ (REST API over HTTPS with cpp-httplib):
1.  This code requires a C++11 compiler (like g++) and the OpenSSL library.
2.  You need the 'httplib.h' header file. Download it from the official repo:
    https://github.com/yhirose/cpp-httplib/blob/master/httplib.h
    Place 'httplib.h' in the same directory as this source file.
3.  Generate a self-signed SSL certificate for the server. In your terminal, run:
    openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha256 -days 365 -nodes -subj "/CN=localhost"
4.  Compile the code. You must link against OpenSSL libraries.
    g++ -std=c++11 -o task108 Task108.cpp -lssl -lcrypto -lpthread
5.  Start the server:
    ./task108 server
6.  In another terminal, run the client to execute test cases:
    ./task108 client
================================================================================
*/
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h" // Must be downloaded and placed in the same directory
#include <iostream>
#include <map>
#include <mutex>
#include <string>

// --- Shared Configuration ---
const char* HOST = "localhost";
const int PORT = 8443;
const char* CERT_FILE = "cert.pem";
const char* KEY_FILE = "key.pem";
const std::string SECRET_TOKEN = "secret-cpp-token-123";

// --- Server Implementation ---
std::map<std::string, std::string> data_store;
std::mutex store_mutex;

void start_server() {
    httplib::SSLServer svr(CERT_FILE, KEY_FILE);

    if (!svr) {
        std::cerr << "Error: Could not create SSL server. Check certificate paths." << std::endl;
        return;
    }
    
    // Authentication middleware
    svr.set_pre_routing_handler([&](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_header("Authorization")) {
            res.set_content("Authorization header missing", "text/plain");
            res.status = 403;
            return httplib::Server::HandlerResponse::Handled;
        }
        std::string auth_header = req.get_header_value("Authorization");
        std::string expected = "Bearer " + SECRET_TOKEN;
        if (auth_header != expected) {
            res.set_content("Invalid token", "text/plain");
            res.status = 403;
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // GET /objects/<key>
    svr.Get(R"(/objects/(\w+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string key = req.matches[1];
        std::lock_guard<std::mutex> lock(store_mutex);
        if (data_store.count(key)) {
            res.set_content(data_store[key], "text/plain");
        } else {
            res.status = 404;
            res.set_content("Not Found", "text/plain");
        }
    });

    // PUT /objects/<key>
    svr.Put(R"(/objects/(\w+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string key = req.matches[1];
        std::lock_guard<std::mutex> lock(store_mutex);
        data_store[key] = req.body;
        res.status = 200;
        res.set_content("OK", "text/plain");
    });

    // DELETE /objects/<key>
    svr.Delete(R"(/objects/(\w+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string key = req.matches[1];
        std::lock_guard<std::mutex> lock(store_mutex);
        if (data_store.count(key)) {
            std::string value = data_store[key];
            data_store.erase(key);
            res.set_content(value, "text/plain");
        } else {
            res.status = 404;
            res.set_content("Not Found", "text/plain");
        }
    });
    
    std::cout << "Starting secure C++ server on https://" << HOST << ":" << PORT << std::endl;
    svr.listen(HOST, PORT);
}

// --- Client Implementation ---
void run_client_tests() {
    std::cout << "Running client tests..." << std::endl;
    httplib::SSLClient cli(HOST, PORT);
    cli.set_ca_cert_path(CERT_FILE);
    cli.enable_server_certificate_verification(true);

    httplib::Headers headers = {
        { "Authorization", "Bearer " + SECRET_TOKEN }
    };
    
    // Test Case 1: Put a new key-value pair
    std::cout << "\n--- Test Case 1: Put new item ---" << std::endl;
    auto res1 = cli.Put("/objects/user1", headers, "data123", "text/plain");
    std::cout << "PUT /objects/user1 -> Status: " << res1->status << std::endl;
    assert(res1 && res1->status == 200);

    // Test Case 2: Get the value for the new key
    std::cout << "\n--- Test Case 2: Get new item ---" << std::endl;
    auto res2 = cli.Get("/objects/user1", headers);
    std::cout << "GET /objects/user1 -> Status: " << res2->status << ", Body: " << res2->body << std::endl;
    assert(res2 && res2->status == 200 && res2->body == "data123");

    // Test Case 3: Update the value for an existing key
    std::cout << "\n--- Test Case 3: Update existing item ---" << std::endl;
    auto res3 = cli.Put("/objects/user1", headers, "data456", "text/plain");
    std::cout << "PUT /objects/user1 -> Status: " << res3->status << std::endl;
    assert(res3 && res3->status == 200);

    // Test Case 4: Get the updated value
    std::cout << "\n--- Test Case 4: Get updated item ---" << std::endl;
    auto res4 = cli.Get("/objects/user1", headers);
    std::cout << "GET /objects/user1 -> Status: " << res4->status << ", Body: " << res4->body << std::endl;
    assert(res4 && res4->status == 200 && res4->body == "data456");

    // Test Case 5: Delete the key
    std::cout << "\n--- Test Case 5: Delete item ---" << std::endl;
    auto res5 = cli.Delete("/objects/user1", headers);
    std::cout << "DELETE /objects/user1 -> Status: " << res5->status << ", Body: " << res5->body << std::endl;
    assert(res5 && res5->status == 200 && res5->body == "data456");
    auto res_after_del = cli.Get("/objects/user1", headers);
    std::cout << "GET /objects/user1 after delete -> Status: " << res_after_del->status << std::endl;
    assert(res_after_del && res_after_del->status == 404);

    // Test Case 6: Authentication Failure
    std::cout << "\n--- Test Case 6: Authentication Failure ---" << std::endl;
    httplib::Headers bad_headers = {
        { "Authorization", "Bearer wrong-token" }
    };
    auto res6 = cli.Get("/objects/user1", bad_headers);
    std::cout << "GET with bad token -> Status: " << res6->status << std::endl;
    assert(res6 && res6->status == 403);

    std::cout << "\nAll client tests completed." << std::endl;
}

int main(int argc, char **argv) {
    if (argc < 2 || (std::string(argv[1]) != "server" && std::string(argv[1]) != "client")) {
        std::cerr << "Usage: " << argv[0] << " <server|client>" << std::endl;
        return 1;
    }

    if (std::string(argv[1]) == "server") {
        start_server();
    } else {
        run_client_tests();
    }

    return 0;
}