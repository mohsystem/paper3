/*
 * This code simulates the backend logic of a web route that processes a JSON payload.
 * It is designed to be run as a single file.
 *
 * Dependency: nlohmann/json.hpp. A single-header JSON library for C++.
 * Download it from: https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp
 * Place `json.hpp` in your include path or in the same directory as this file.
 */
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

// Include the single-header JSON library
#include "json.hpp"

// For convenience, use a namespace alias
using json = nlohmann::json;

/**
 * Decodes a URL-encoded string.
 * @param encoded The string to decode.
 * @return The decoded string.
 */
std::string urlDecode(const std::string& encoded) {
    std::string decoded;
    decoded.reserve(encoded.length());
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            try {
                std::string hex = encoded.substr(i + 1, 2);
                char c = static_cast<char>(std::stoi(hex, nullptr, 16));
                decoded += c;
                i += 2;
            } catch (const std::invalid_argument& e) {
                decoded += encoded[i]; // Append '%' if hex is invalid
            } catch (const std::out_of_range& e) {
                decoded += encoded[i]; // Append '%' if hex is out of range
            }
        } else if (encoded[i] == '+') {
            decoded += ' ';
        } else {
            decoded += encoded[i];
        }
    }
    return decoded;
}

/**
 * Simulates processing a request to an /api endpoint. It extracts a 'payload'
 * parameter from the query string, parses it as JSON, and returns the value 
 * associated with the 'name' key.
 *
 * @param queryString The query string from the URL (e.g., "payload=%7B...%7D").
 * @return The value of the 'name' key if found, otherwise an error message.
 */
std::string processApiRequest(const std::string& queryString) {
    const std::string payload_key = "payload=";
    size_t start_pos = queryString.find(payload_key);

    if (start_pos == std::string::npos) {
        return "Error: 'payload' parameter not found.";
    }

    std::string payloadEncoded = queryString.substr(start_pos + payload_key.length());
    // Find the end of the payload value (bounded by '&' or end of string)
    size_t end_pos = payloadEncoded.find('&');
    if (end_pos != std::string::npos) {
        payloadEncoded = payloadEncoded.substr(0, end_pos);
    }
    
    if (payloadEncoded.empty()) {
        return "Error: 'payload' parameter is empty.";
    }

    std::string payloadDecoded = urlDecode(payloadEncoded);

    try {
        json data = json::parse(payloadDecoded);

        if (!data.is_object()) {
            return "Error: Payload is not a JSON object.";
        }

        if (!data.contains("name")) {
            return "Error: 'name' key not found in payload.";
        }
        
        if (!data["name"].is_string()) {
            return "Error: 'name' value is not a string.";
        }

        return data["name"].get<std::string>();

    } catch (json::parse_error& e) {
        return "Error: Malformed JSON in payload.";
    } catch (const std::exception& e) {
        return std::string("Error: An unexpected error occurred: ") + e.what();
    }
}


int main() {
    std::vector<std::string> testCases = {
        // 1. Valid case with a 'name' key
        "payload=%7B%22name%22%3A%22Alice%22%2C%20%22id%22%3A123%7D",
        // 2. Valid JSON but missing the 'name' key
        "payload=%7B%22id%22%3A456%7D",
        // 3. Malformed JSON (extra comma)
        "payload=%7B%22name%22%3A%22Bob%22%2C%7D",
        // 4. Query string without a 'payload' parameter
        "other_param=abc&another=123",
        // 5. 'name' key exists but its value is not a string
        "payload=%7B%22name%22%3A12345%7D"
    };

    std::cout << "Running test cases..." << std::endl;
    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Input: " << testCases[i] << std::endl;
        std::string result = processApiRequest(testCases[i]);
        std::cout << "Output: " << result << std::endl;
    }

    return 0;
}