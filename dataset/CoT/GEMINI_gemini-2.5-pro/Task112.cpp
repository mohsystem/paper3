/*
 * PRE-REQUISITES:
 * 1. Install the MongoDB C++ driver. Follow instructions at:
 *    http://mongocxx.org/mongocxx-v3/installation/
 * 2. This code must be compiled with the C++17 standard or newer.
 *
 * COMPILE & RUN (example on Linux/macOS):
 * g++ -std=c++17 Task112.cpp -o Task112 \
 *   $(pkg-config --cflags --libs libmongocxx)
 *
 * ./Task112
 */
#include <iostream>
#include <string>
#include <vector>
#include <optional>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/exception/exception.hpp>

using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;

class MongoCRUD {
public:
    MongoCRUD(const std::string& uri_string, const std::string& db_name, const std::string& collection_name) {
        try {
            uri = mongocxx::uri{uri_string};
            client = mongocxx::client{uri};
            db = client[db_name];
            coll = db[collection_name];
        } catch (const mongocxx::exception& e) {
            std::cerr << "MongoDB connection error: " << e.what() << std::endl;
            throw;
        }
    }

    std::optional<bsoncxx::oid> createUser(bsoncxx::v_noabi::document::view doc) {
        try {
            auto result = coll.insert_one(doc);
            if (result) {
                return result->inserted_id().get_oid().value;
            }
        } catch (const mongocxx::exception& e) {
            std::cerr << "Error creating user: " << e.what() << std::endl;
        }
        return std::nullopt;
    }

    std::optional<bsoncxx::document::value> readUser(bsoncxx::v_noabi::document::view filter) {
        try {
            return coll.find_one(filter);
        } catch (const mongocxx::exception& e) {
            std::cerr << "Error reading user: " << e.what() << std::endl;
        }
        return std::nullopt;
    }

    std::optional<int64_t> updateUser(bsoncxx::v_noabi::document::view filter, bsoncxx::v_noabi::document::view update) {
        try {
            auto result = coll.update_one(filter, update);
            if (result) {
                return result->modified_count();
            }
        } catch (const mongocxx::exception& e) {
            std::cerr << "Error updating user: " << e.what() << std::endl;
        }
        return std::nullopt;
    }

    std::optional<int64_t> deleteUser(bsoncxx::v_noabi::document::view filter) {
        try {
            auto result = coll.delete_one(filter);
            if (result) {
                return result->deleted_count();
            }
        } catch (const mongocxx::exception& e) {
            std::cerr << "Error deleting user: " << e.what() << std::endl;
        }
        return std::nullopt;
    }

    void dropCollection() {
        try {
            coll.drop();
        } catch (const mongocxx::exception& e) {
             std::cerr << "Error dropping collection: " << e.what() << std::endl;
        }
    }

private:
    mongocxx::uri uri;
    mongocxx::client client;
    mongocxx::database db;
    mongocxx::collection coll;
};

int main() {
    // The mongocxx::instance constructor and destructor initialize and shut down the driver.
    // It must be created before any other C++ driver functions are called.
    mongocxx::instance instance{};

    try {
        MongoCRUD crud("mongodb://localhost:27017", "cpp_test_db", "users");

        // Clean up collection before tests
        crud.dropCollection();
        std::cout << "--- Starting MongoDB CRUD Test Cases ---" << std::endl;

        // Test Case 1: Create
        std::cout << "\n--- Test Case 1: Create User ---" << std::endl;
        auto doc_builder = document{};
        bsoncxx::v_noabi::document::value doc_to_insert = doc_builder
            << "name" << "Peter Jones"
            << "age" << 42
            << "city" << "Chicago"
            << finalize;
        auto user_id_opt = crud.createUser(doc_to_insert.view());
        if (!user_id_opt) {
            return 1;
        }
        bsoncxx::oid user_id = user_id_opt.value();
        std::cout << "User created successfully with ID: " << user_id.to_string() << std::endl;

        // Test Case 2: Read
        std::cout << "\n--- Test Case 2: Read User ---" << std::endl;
        auto read_filter = document{} << "_id" << user_id << finalize;
        auto found_user_opt = crud.readUser(read_filter.view());
        if (found_user_opt) {
            std::cout << "User found: " << bsoncxx::to_json(found_user_opt.value().view()) << std::endl;
        } else {
            std::cout << "User not found." << std::endl;
        }

        // Test Case 3: Update
        std::cout << "\n--- Test Case 3: Update User ---" << std::endl;
        auto update_doc = document{} << "$set" << open_document << "age" << 43 << close_document << finalize;
        auto modified_count_opt = crud.updateUser(read_filter.view(), update_doc.view());
        if (modified_count_opt) {
            std::cout << "Users updated: " << modified_count_opt.value() << std::endl;
        }

        // Test Case 4: Read after Update
        std::cout << "\n--- Test Case 4: Read User After Update ---" << std::endl;
        auto updated_user_opt = crud.readUser(read_filter.view());
        if (updated_user_opt) {
            std::cout << "User found after update: " << bsoncxx::to_json(updated_user_opt.value().view()) << std::endl;
        } else {
            std::cout << "User not found." << std::endl;
        }

        // Test Case 5: Delete
        std::cout << "\n--- Test Case 5: Delete User ---" << std::endl;
        auto deleted_count_opt = crud.deleteUser(read_filter.view());
        if (deleted_count_opt) {
            std::cout << "Users deleted: " << deleted_count_opt.value() << std::endl;
        }
        auto deleted_user_opt = crud.readUser(read_filter.view());
        if (!deleted_user_opt) {
            std::cout << "User successfully deleted and not found." << std::endl;
        } else {
            std::cout << "User deletion failed." << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n--- Tests Finished ---" << std::endl;
    return 0;
}