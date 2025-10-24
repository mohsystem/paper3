/*
 * To compile and run this code, you need the MongoDB C++ driver (mongocxx).
 * You also need its dependency, the BSON C++ library (bsoncxx).
 *
 * Example compilation command using pkg-config:
 * g++ -std=c++17 -o task112_cpp task112.cpp $(pkg-config --cflags --libs libmongocxx)
 *
 * Make sure you have a MongoDB instance running on localhost:27017.
 */
#include <iostream>
#include <string>
#include <optional>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/exception/exception.hpp>

// NOTE: For a production environment, the connection string should not be hardcoded.
// It should be loaded from a secure configuration source or environment variables.
const mongocxx::uri MONGODB_URI{"mongodb://localhost:27017"};
const std::string DATABASE_NAME = "my_cpp_db";
const std::string COLLECTION_NAME = "users";

/**
 * Creates a new document in the specified collection.
 * @param collection The MongoDB collection.
 * @param name The name of the user.
 * @param email The email of the user.
 * @return The ObjectId of the inserted document as a hex string, or an empty optional on failure.
 */
std::optional<std::string> createUser(mongocxx::collection& collection, const std::string& name, const std::string& email) {
    if (name.empty() || email.empty()) {
        std::cerr << "Error: Name and email cannot be empty." << std::endl;
        return std::nullopt;
    }
    try {
        bsoncxx::builder::stream::document builder{};
        builder << "name" << name << "email" << email;
        auto result = collection.insert_one(builder.view());
        if (result) {
            return result->inserted_id().get_oid().value.to_string();
        }
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error creating user: " << e.what() << std::endl;
    }
    return std::nullopt;
}

/**
 * Reads a document from the collection by name.
 * @param collection The MongoDB collection.
 * @param name The name of the user to find.
 * @return The found document as a BSON value, or an empty optional if not found or on error.
 */
std::optional<bsoncxx::document::value> readUserByName(mongocxx::collection& collection, const std::string& name) {
    if (name.empty()) {
        std::cerr << "Error: Name cannot be empty." << std::endl;
        return std::nullopt;
    }
    try {
        bsoncxx::builder::stream::document filter_builder{};
        filter_builder << "name" << name;
        return collection.find_one(filter_builder.view());
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error reading user: " << e.what() << std::endl;
        return std::nullopt;
    }
}

/**
 * Updates a user's email by their name.
 * @param collection The MongoDB collection.
 * @param name The name of the user to update.
 * @param newEmail The new email address.
 * @return true if the update was successful, false otherwise.
 */
bool updateUserEmail(mongocxx::collection& collection, const std::string& name, const std::string& newEmail) {
    if (name.empty() || newEmail.empty()) {
        std::cerr << "Error: Name and new email cannot be empty." << std::endl;
        return false;
    }
    try {
        bsoncxx::builder::stream::document filter_builder{}, update_builder{};
        filter_builder << "name" << name;
        update_builder << "$set" << bsoncxx::builder::stream::open_document << "email" << newEmail << bsoncxx::builder::stream::close_document;
        
        auto result = collection.update_one(filter_builder.view(), update_builder.view());
        return result && result->modified_count() > 0;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error updating user: " << e.what() << std::endl;
        return false;
    }
}

/**
 * Deletes a user by their name.
 * @param collection The MongoDB collection.
 * @param name The name of the user to delete.
 * @return true if the deletion was successful, false otherwise.
 */
bool deleteUser(mongocxx::collection& collection, const std::string& name) {
    if (name.empty()) {
        std::cerr << "Error: Name cannot be empty." << std::endl;
        return false;
    }
    try {
        bsoncxx::builder::stream::document filter_builder{};
        filter_builder << "name" << name;
        auto result = collection.delete_one(filter_builder.view());
        return result && result->deleted_count() > 0;
    } catch (const mongocxx::exception& e) {
        std::cerr << "Error deleting user: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    // The mongocxx::instance constructor and destructor initialize and shut down the driver.
    // It must be created before any other mongocxx objects and remain alive for their entire lifetime.
    mongocxx::instance inst{};

    try {
        mongocxx::client client{MONGODB_URI};
        auto db = client[DATABASE_NAME];
        auto collection = db[COLLECTION_NAME];

        // Clean up the collection before running tests
        collection.drop();
        std::cout << "Collection dropped for a clean test run." << std::endl;
        
        std::cout << "--- MongoDB CRUD Operations in C++ ---" << std::endl;

        // 1. Create Operation
        std::cout << "\n[Test Case 1: Create]" << std::endl;
        auto new_user_id = createUser(collection, "CPP User", "cpp.user@example.com");
        if (new_user_id) {
            std::cout << "User created successfully with ID: " << *new_user_id << std::endl;
        } else {
            std::cout << "User creation failed." << std::endl;
        }

        // 2. Read Operation
        std::cout << "\n[Test Case 2: Read]" << std::endl;
        auto user = readUserByName(collection, "CPP User");
        if (user) {
            std::cout << "User found: " << bsoncxx::to_json(user->view()) << std::endl;
        } else {
            std::cout << "User 'CPP User' not found." << std::endl;
        }

        // 3. Update Operation
        std::cout << "\n[Test Case 3: Update]" << std::endl;
        bool updated = updateUserEmail(collection, "CPP User", "cpp.new@example.com");
        if (updated) {
            std::cout << "User 'CPP User' updated successfully." << std::endl;
        } else {
            std::cout << "User update failed." << std::endl;
        }
        
        // 4. Read again to verify update
        std::cout << "\n[Test Case 4: Verify Update]" << std::endl;
        user = readUserByName(collection, "CPP User");
        if (user) {
            std::cout << "User found after update: " << bsoncxx::to_json(user->view()) << std::endl;
        } else {
            std::cout << "User 'CPP User' not found after update." << std::endl;
        }
        
        // 5. Delete Operation
        std::cout << "\n[Test Case 5: Delete]" << std::endl;
        bool deleted = deleteUser(collection, "CPP User");
        if (deleted) {
            std::cout << "User 'CPP User' deleted successfully." << std::endl;
        } else {
            std::cout << "User deletion failed." << std::endl;
        }

        // Verify deletion
        user = readUserByName(collection, "CPP User");
        if (!user) {
            std::cout << "Successfully verified that 'CPP User' is no longer in the database." << std::endl;
        } else {
            std::cout << "Verification failed: 'CPP User' still exists." << std::endl;
        }

    } catch (const mongocxx::exception& e) {
        std::cerr << "An error occurred with MongoDB connection: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}