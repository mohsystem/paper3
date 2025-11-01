#include <iostream>
#include <string>
#include <vector>
#include <map>

// Simulating a database table with a Map
static std::map<std::string, std::string> imageDatabase;

/**
 * Encodes a string of raw data into a Base64 string.
 * @param in The input string (can contain any byte data).
 * @return The Base64 encoded string.
 */
std::string base64_encode(const std::string& in) {
    std::string out;
    const std::string b64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64_table[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(b64_table[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (out.size() % 4) {
        out.push_back('=');
    }
    return out;
}

/**
 * Simulates reading image data, converting it to base64, and inserting it into a database.
 * @param imageName The name of the image file.
 * @param imageData The raw byte data of the image, represented as a string.
 */
void upload_image(const std::string& imageName, const std::string& imageData) {
    if (imageName.empty()) {
        std::cout << "Error: Image name cannot be empty." << std::endl;
        return;
    }

    // Convert image data to Base64 string
    std::string base64ImageString = base64_encode(imageData);

    // Insert image name and base64 string into the database (simulated)
    // If the name already exists, its value is updated.
    imageDatabase[imageName] = base64ImageString;

    std::cout << "Successfully uploaded and stored '" << imageName << "'." << std::endl;
}

/**
 * Displays the current contents of the simulated database.
 */
void displayDatabaseContents() {
    std::cout << "\n--- Simulated Database Contents ---" << std::endl;
    if (imageDatabase.empty()) {
        std::cout << "Database is empty." << std::endl;
    } else {
        for (const auto& pair : imageDatabase) {
            std::cout << "Image Name: " << pair.first << std::endl;
            std::string snippet = pair.second.substr(0, 30);
            if (pair.second.length() > 30) {
                snippet += "...";
            }
            std::cout << "Base64 Data (Snippet): " << snippet << std::endl;
            std::cout << "---------------------------------" << std::endl;
        }
    }
}

int main() {
    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    // Test Case 1: A simple text as image data
    upload_image("profile_pic.jpg", "This is my profile picture content.");

    // Test Case 2: Another text file
    upload_image("document.png", "Important document data here.");

    // Test Case 3: Empty image data
    upload_image("empty_image.gif", "");

    // Test Case 4: Image with special characters
    upload_image("art-@#$.svg", "SVG content with <tags> & symbols!");

    // Test Case 5: Overwriting an existing image
    upload_image("profile_pic.jpg", "This is my NEW profile picture.");

    // Display the final state of the database
    displayDatabaseContents();

    return 0;
}