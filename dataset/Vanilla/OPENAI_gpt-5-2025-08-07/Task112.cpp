#include <iostream>
#include <string>
#include <vector>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/result/insert_one.hpp>
#include <mongocxx/result/update.hpp>
#include <mongocxx/result/delete.hpp>

bool create_document(mongocxx::collection& collection, const std::string& id, const std::string& name, int age) {
    try {
        auto doc = bsoncxx::builder::stream::document{}
                   << "_id" << id
                   << "name" << name
                   << "age" << age
                   << bsoncxx::builder::stream::finalize;
        auto res = collection.insert_one(doc.view());
        return res && res->result().inserted_count() == 1;
    } catch (...) {
        return false;
    }
}

std::string read_document(mongocxx::collection& collection, const std::string& id) {
    try {
        auto filter = bsoncxx::builder::stream::document{} << "_id" << id << bsoncxx::builder::stream::finalize;
        auto doc = collection.find_one(filter.view());
        if (doc) {
            return bsoncxx::to_json(*doc);
        }
        return std::string();
    } catch (...) {
        return std::string();
    }
}

bool update_document(mongocxx::collection& collection, const std::string& id, const std::string& new_name, int new_age) {
    try {
        auto filter = bsoncxx::builder::stream::document{} << "_id" << id << bsoncxx::builder::stream::finalize;
        auto update = bsoncxx::builder::stream::document{}
                      << "$set" << bsoncxx::builder::stream::open_document
                      << "name" << new_name
                      << "age" << new_age
                      << bsoncxx::builder::stream::close_document
                      << bsoncxx::builder::stream::finalize;
        auto res = collection.update_one(filter.view(), update.view());
        return res && res->modified_count() > 0;
    } catch (...) {
        return false;
    }
}

bool delete_document(mongocxx::collection& collection, const std::string& id) {
    try {
        auto filter = bsoncxx::builder::stream::document{} << "_id" << id << bsoncxx::builder::stream::finalize;
        auto res = collection.delete_one(filter.view());
        return res && res->deleted_count() > 0;
    } catch (...) {
        return false;
    }
}

int main() {
    mongocxx::instance instance{};
    mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};
    auto db = client["task112db"];
    auto collection = db["cpp_people"];

    // Cleanup
    collection.delete_many(bsoncxx::builder::stream::document{} << "_id" << bsoncxx::builder::stream::open_document
                           << "$in" << bsoncxx::builder::stream::open_array << "u1" << "u2" << "u3" << "u4" << "u5"
                           << bsoncxx::builder::stream::close_array
                           << bsoncxx::builder::stream::close_document
                           << bsoncxx::builder::stream::finalize);

    // Test case 1: Create
    std::cout << "Create u1: " << (create_document(collection, "u1", "Alice", 30) ? "true" : "false") << std::endl;

    // Test case 2: Read
    std::string r = read_document(collection, "u1");
    std::cout << "Read u1: " << (r.empty() ? "null" : r) << std::endl;

    // Test case 3: Update
    std::cout << "Update u1: " << (update_document(collection, "u1", "Alicia", 31) ? "true" : "false") << std::endl;
    std::string r2 = read_document(collection, "u1");
    std::cout << "Read u1 after update: " << (r2.empty() ? "null" : r2) << std::endl;

    // Test case 4: Create another
    std::cout << "Create u2: " << (create_document(collection, "u2", "Bob", 25) ? "true" : "false") << std::endl;

    // Test case 5: Delete
    std::cout << "Delete u2: " << (delete_document(collection, "u2") ? "true" : "false") << std::endl;

    return 0;
}