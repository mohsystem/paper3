#include <iostream>
#include <vector>
#include <optional>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

/*
NOTE: To compile this code, you need the MongoDB C++ driver.
Installation instructions: http://mongocxx.org/mongocxx-v3/installation/

Example compilation command (after installing the driver):
g++ -o Task112 Task112.cpp -std=c++17 \
    $(pkg-config --cflags --libs libmongocxx)
*/

using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;

// CREATE
void createDocument(mongocxx::collection& collection, const bsoncxx::document::view& doc) {
    try {
        mongocxx::stdx::optional<mongocxx::result::insert_one> result = collection.insert_one(doc);
        if (result) {
            std::cout << "Success: Document inserted with id: " << (*result).inserted_id().get_oid().value.to_string() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error inserting document: " << e.what() << std::endl;
    }
}

// READ
std::optional<bsoncxx::document::value> readDocument(mongocxx::collection& collection, const bsoncxx::document::view& filter) {
    try {
        return collection.find_one(filter);
    } catch (const std::exception& e) {
        std::cerr << "Error reading document: " << e.what() << std::endl;
        return std::nullopt;
    }
}

// UPDATE
void updateDocument(mongocxx::collection& collection, const bsoncxx::document::view& filter, const bsoncxx::document::view& update) {
    try {
        mongocxx::stdx::optional<mongocxx::result::update> result = collection.update_one(filter, update);
        if (result) {
            std::cout << "Success: Matched " << result->matched_count() << " and modified " << result->modified_count() << " document(s)." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error updating document: " << e.what() << std::endl;
    }
}

// DELETE
void deleteDocument(mongocxx::collection& collection, const bsoncxx::document::view& filter) {
    try {
        mongocxx::stdx::optional<mongocxx::result::delete_result> result = collection.delete_one(filter);
        if (result) {
            std::cout << "Success: Deleted " << result->deleted_count() << " document(s)." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error deleting document: " << e.what() << std::endl;
    }
}


int main() {
    // The mongocxx::instance constructor and destructor initialize and shut down the driver,
    // respectively. Therefore, a mongocxx::instance object should be created before using the driver and
    // must remain alive for as long as the driver is in use.
    mongocxx::instance instance{};
    
    try {
        const auto uri = mongocxx::uri{"mongodb://localhost:27017"};
        mongocxx::client client{uri};
        auto db = client["cppTestDB"];
        auto collection = db["users"];
        
        // Clean up previous runs
        collection.drop();

        std::cout << "--- Starting CRUD Test Cases ---" << std::endl;
        
        // --- Test Case 1: Create ---
        std::cout << "\n1. CREATE Operation" << std::endl;
        auto doc_builder = document{};
        bsoncxx::document::value doc_to_create = doc_builder
            << "name" << "Grace Hopper"
            << "age" << 85
            << "profession" << "Computer Scientist"
            << finalize;
        createDocument(collection, doc_to_create.view());

        // --- Test Case 2: Read ---
        std::cout << "\n2. READ Operation" << std::endl;
        auto filter_builder_read = document{};
        filter_builder_read << "name" << "Grace Hopper";
        auto found_doc_opt = readDocument(collection, filter_builder_read.view());
        if (found_doc_opt) {
            std::cout << "Found document: " << bsoncxx::to_json(found_doc_opt->view()) << std::endl;
        } else {
            std::cout << "Document not found." << std::endl;
        }

        // --- Test Case 3: Update ---
        std::cout << "\n3. UPDATE Operation" << std::endl;
        auto filter_builder_update = document{};
        filter_builder_update << "name" << "Grace Hopper";
        auto update_builder = document{};
        update_builder << "$set" << open_document << "age" << 90 << close_document;
        updateDocument(collection, filter_builder_update.view(), update_builder.view());

        // --- Test Case 4: Read after Update ---
        std::cout << "\n4. READ Operation (after update)" << std::endl;
        auto updated_doc_opt = readDocument(collection, filter_builder_read.view());
         if (updated_doc_opt) {
            std::cout << "Found updated document: " << bsoncxx::to_json(updated_doc_opt->view()) << std::endl;
        } else {
            std::cout << "Document not found." << std::endl;
        }

        // --- Test Case 5: Delete ---
        std::cout << "\n5. DELETE Operation" << std::endl;
        auto filter_builder_delete = document{};
        filter_builder_delete << "name" << "Grace Hopper";
        deleteDocument(collection, filter_builder_delete.view());
        auto deleted_doc_opt = readDocument(collection, filter_builder_delete.view());
        std::cout << "Document after deletion attempt: " << (deleted_doc_opt ? "Found" : "Not Found") << std::endl;

        std::cout << "\n--- CRUD Test Cases Finished ---" << std::endl;

        // --- Final Cleanup ---
        std::cout << "Dropping collection..." << std::endl;
        collection.drop();
        std::cout << "Collection dropped." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}