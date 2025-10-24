#include <iostream>
#include <string>
#include <vector>
#include <optional>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

/**
 * Note: To compile this code, you need the MongoDB C++ driver (mongocxx).
 * Installation instructions: http://mongocxx.org/mongocxx-v3/installation/
 * 
 * Example compile command on Linux:
 * g++ -o task112_cpp task112.cpp -I/usr/local/include/mongocxx/v_noabi \
 * -I/usr/local/include/bsoncxx/v_noabi -L/usr/local/lib -lmongocxx -lbsoncxx
 * 
 * The paths might vary based on your installation.
 */

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

class MongoCRUD {
private:
    mongocxx::client client;
    mongocxx::database db;
    mongocxx::collection coll;

public:
    MongoCRUD(const mongocxx::uri& uri, const std::string& db_name, const std::string& coll_name)
        : client(uri), db(client[db_name]), coll(db[coll_name]) {}

    std::optional<bsoncxx::oid> createDocument(const std::string& name, int age, const std::string& city) {
        try {
            auto builder = document{};
            bsoncxx::document::value doc_value = builder
                << "name" << name
                << "age" << age
                << "city" << city
                << finalize;

            bsoncxx::stdx::optional<mongocxx::result::insert_one> result = coll.insert_one(doc_value.view());
            if (result) {
                bsoncxx::oid oid = result->inserted_id().get_oid().value;
                std::cout << "Create: Success. Inserted document with id: " << oid.to_string() << std::endl;
                return oid;
            }
        } catch (const mongocxx::exception& e) {
            std::cerr << "An error occurred during create operation: " << e.what() << std::endl;
        }
        return std::nullopt;
    }

    std::optional<bsoncxx::document::value> readDocument(const bsoncxx::oid& id) {
        try {
            bsoncxx::stdx::optional<bsoncxx::document::value> result =
                coll.find_one(document{} << "_id" << id << finalize);
            if (result) {
                std::cout << "Read: Success. Found document: " << bsoncxx::to_json(result->view()) << std::endl;
                return result;
            } else {
                std::cout << "Read: Document with id " << id.to_string() << " not found." << std::endl;
            }
        } catch (const mongocxx::exception& e) {
            std::cerr << "An error occurred during read operation: " << e.what() << std::endl;
        }
        return std::nullopt;
    }

    bool updateDocument(const bsoncxx::oid& id, const std::string& field, int new_value) {
        try {
            bsoncxx::stdx::optional<mongocxx::result::update> result = coll.update_one(
                document{} << "_id" << id << finalize,
                document{} << "$set" << open_document << field << new_value << close_document << finalize);

            if (result) {
                std::cout << "Update: Success. Matched " << result->matched_count() << " and modified " << result->modified_count() << " document(s)." << std::endl;
                return result->modified_count() > 0;
            }
        } catch (const mongocxx::exception& e) {
            std::cerr << "An error occurred during update operation: " << e.what() << std::endl;
        }
        return false;
    }

    bool deleteDocument(const bsoncxx::oid& id) {
        try {
            bsoncxx::stdx::optional<mongocxx::result::delete_result> result =
                coll.delete_one(document{} << "_id" << id << finalize);

            if (result) {
                std::cout << "Delete: Success. Deleted " << result->deleted_count() << " document(s)." << std::endl;
                return result->deleted_count() > 0;
            }
        } catch (const mongocxx::exception& e) {
            std::cerr << "An error occurred during delete operation: " << e.what() << std::endl;
        }
        return false;
    }
    
    void cleanup() {
        try {
            coll.drop();
        } catch (const mongocxx::exception& e) {
            // It's okay if collection doesn't exist
        }
    }
};

int main() {
    // This should be done only once.
    mongocxx::instance instance{};

    // IMPORTANT: In a production environment, connection strings should never be hard-coded.
    // They should be retrieved from a secure source like environment variables or a secrets manager.
    const auto uri = mongocxx::uri{"mongodb://localhost:27017"};
    const std::string db_name = "mySecureDb";
    const std::string coll_name = "users";

    try {
        MongoCRUD crud_ops(uri, db_name, coll_name);
        std::cout << "Successfully connected to MongoDB." << std::endl;

        // Cleanup before tests
        crud_ops.cleanup();
        std::cout << "--- Starting Test Cases ---" << std::endl;
        
        // --- Test Case 1: Create a new user ---
        std::cout << "\n1. === CREATE Operation ===" << std::endl;
        auto user_id_opt = crud_ops.createDocument("Carol Danvers", 35, "New York");

        if(user_id_opt) {
            bsoncxx::oid user_id = *user_id_opt;

            // --- Test Case 2: Read the created user ---
            std::cout << "\n2. === READ Operation (after create) ===" << std::endl;
            crud_ops.readDocument(user_id);

            // --- Test Case 3: Update the user's age ---
            std::cout << "\n3. === UPDATE Operation ===" << std::endl;
            crud_ops.updateDocument(user_id, "age", 36);

            // --- Test Case 4: Read the updated user ---
            std::cout << "\n4. === READ Operation (after update) ===" << std::endl;
            crud_ops.readDocument(user_id);

            // --- Test Case 5: Delete the user ---
            std::cout << "\n5. === DELETE Operation ===" << std::endl;
            crud_ops.deleteDocument(user_id);

            // --- Final check: try to read the deleted user ---
            std::cout << "\n--- Final Check: Verifying Deletion ---" << std::endl;
            crud_ops.readDocument(user_id);
        } else {
             std::cout << "Test cases aborted due to creation failure." << std::endl;
        }

        std::cout << "\n--- Test Cases Finished ---" << std::endl;

    } catch (const mongocxx::exception& e) {
        std::cerr << "Connection or operation failed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}