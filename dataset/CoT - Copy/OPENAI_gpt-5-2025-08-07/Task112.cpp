#include <iostream>
#include <vector>
#include <cstdlib>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/options/client.hpp>
#include <mongocxx/options/acknowledge.hpp>
#include <mongocxx/exception/exception.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

mongocxx::client create_client() {
    const char* env_uri = std::getenv("MONGODB_URI");
    std::string uri = env_uri && std::string(env_uri).size() > 0
                      ? std::string(env_uri)
                      : std::string("mongodb://localhost:27017/?retryWrites=true&w=majority&serverSelectionTimeoutMS=10000&connectTimeoutMS=10000");
    mongocxx::uri u(uri);
    mongocxx::options::client opts;
    // retryWrites derives from URI; additional options can be added here if needed
    return mongocxx::client(u, opts);
}

mongocxx::collection get_collection(mongocxx::client& client, const std::string& db, const std::string& coll) {
    auto database = client[db];
    auto collection = database[coll];
    return collection;
}

bsoncxx::oid create_document(mongocxx::collection& collection, const bsoncxx::document::view_or_value& doc) {
    auto res = collection.insert_one(doc);
    if (!res) throw std::runtime_error("Insert failed");
    auto id = res->inserted_id();
    if (id.type() == bsoncxx::type::k_oid) {
        return id.get_oid().value;
    } else {
        // If inserted_id is not oid, generate from doc if present
        auto v = doc.view();
        auto el = v["_id"];
        if (el && el.type() == bsoncxx::type::k_oid) {
            return el.get_oid().value;
        }
        throw std::runtime_error("Inserted id is not OID");
    }
}

std::vector<bsoncxx::document::value> read_documents(mongocxx::collection& collection, const bsoncxx::document::view_or_value& filter) {
    std::vector<bsoncxx::document::value> out;
    auto cursor = collection.find(filter);
    for (auto&& doc : cursor) {
        out.emplace_back(bsoncxx::document::value(doc));
    }
    return out;
}

std::int64_t update_documents(mongocxx::collection& collection, const bsoncxx::document::view_or_value& filter, const bsoncxx::document::view_or_value& update) {
    auto res = collection.update_many(filter, update);
    if (!res) return 0;
    return res->modified_count();
}

std::int64_t delete_documents(mongocxx::collection& collection, const bsoncxx::document::view_or_value& filter) {
    auto res = collection.delete_many(filter);
    if (!res) return 0;
    return res->deleted_count();
}

int main() {
    static mongocxx::instance inst{};
    try {
        auto client = create_client();
        auto coll = get_collection(client, "task112db", "users");
        const std::string tag = "task112";

        // Test 1: Cleanup
        auto t1 = delete_documents(coll, document{} << "tag" << tag << finalize);
        std::cout << "Test1-Cleanup deleted: " << t1 << std::endl;

        // Test 2: Create
        bsoncxx::builder::stream::document builder;
        builder << "username" << "alice"
                << "email" << "alice.task112@example.com"
                << "age" << 30
                << "tag" << tag;
        auto oid = create_document(coll, builder.view());
        std::cout << "Test2-Create insertedId: " << oid.to_string() << std::endl;

        // Test 3: Read
        auto docs = read_documents(coll, document{} << "username" << "alice" << "tag" << tag << finalize);
        std::cout << "Test3-Read count: " << docs.size() << std::endl;

        // Test 4: Update
        auto modified = update_documents(
            coll,
            document{} << "username" << "alice" << "tag" << tag << finalize,
            document{} << "$set" << open_document << "age" << 31 << close_document << finalize
        );
        std::cout << "Test4-Update modified: " << modified << std::endl;

        // Test 5: Delete
        auto del = delete_documents(coll, document{} << "username" << "alice" << "tag" << tag << finalize);
        std::cout << "Test5-Delete deleted: " << del << std::endl;

    } catch (const mongocxx::exception& e) {
        std::cerr << "MongoDB error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
    return 0;
}