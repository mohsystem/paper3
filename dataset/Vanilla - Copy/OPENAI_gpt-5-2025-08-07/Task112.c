#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <bson/bson.h>
#include <mongoc/mongoc.h>

// Create a document with fields: _id (string), name (string), age (int)
bool create_document(mongoc_collection_t* collection, const char* id, const char* name, int age) {
    bson_t* doc = bson_new();
    BSON_APPEND_UTF8(doc, "_id", id);
    BSON_APPEND_UTF8(doc, "name", name);
    BSON_APPEND_INT32(doc, "age", age);

    bson_error_t error;
    bool ret = mongoc_collection_insert_one(collection, doc, NULL, NULL, &error);
    if (!ret) {
        // fprintf(stderr, "Insert failed: %s\n", error.message);
    }
    bson_destroy(doc);
    return ret;
}

// Read a document by _id and return its canonical extended JSON string (caller must bson_free the returned string). Returns NULL if not found or on error.
char* read_document(mongoc_collection_t* collection, const char* id) {
    bson_t* filter = bson_new();
    BSON_APPEND_UTF8(filter, "_id", id);

    mongoc_cursor_t* cursor = mongoc_collection_find_with_opts(collection, filter, NULL, NULL);
    const bson_t* doc;
    char* json = NULL;

    if (mongoc_cursor_next(cursor, &doc)) {
        json = bson_as_canonical_extended_json(doc, NULL);
    }

    bson_destroy(filter);
    mongoc_cursor_destroy(cursor);
    return json;
}

// Update a document by _id setting name and age
bool update_document(mongoc_collection_t* collection, const char* id, const char* new_name, int new_age) {
    bson_t* filter = bson_new();
    BSON_APPEND_UTF8(filter, "_id", id);

    bson_t update;
    bson_init(&update);
    bson_t set;
    BSON_APPEND_DOCUMENT_BEGIN(&update, "$set", &set);
    BSON_APPEND_UTF8(&set, "name", new_name);
    BSON_APPEND_INT32(&set, "age", new_age);
    bson_append_document_end(&update, &set);

    bson_error_t error;
    bool ret = mongoc_collection_update_one(collection, filter, &update, NULL, NULL, &error);
    if (!ret) {
        // fprintf(stderr, "Update failed: %s\n", error.message);
    }

    bson_destroy(filter);
    bson_destroy(&update);
    return ret;
}

// Delete a document by _id
bool delete_document(mongoc_collection_t* collection, const char* id) {
    bson_t* filter = bson_new();
    BSON_APPEND_UTF8(filter, "_id", id);

    bson_error_t error;
    bool ret = mongoc_collection_delete_one(collection, filter, NULL, NULL, &error);
    if (!ret) {
        // fprintf(stderr, "Delete failed: %s\n", error.message);
    }

    bson_destroy(filter);
    return ret;
}

int main() {
    mongoc_init();

    const char* uri_string = "mongodb://localhost:27017";
    mongoc_uri_t* uri = mongoc_uri_new_with_error(uri_string, NULL);
    if (!uri) {
        fprintf(stderr, "Failed to parse URI.\n");
        return 1;
    }

    mongoc_client_t* client = mongoc_client_new_from_uri(uri);
    mongoc_database_t* db = mongoc_client_get_database(client, "task112db");
    mongoc_collection_t* collection = mongoc_client_get_collection(client, "task112db", "c_people");

    // Cleanup old test docs
    bson_t* del_filter = BCON_NEW("_id", "{", "$in", "[", BCON_UTF8("u1"), BCON_UTF8("u2"), BCON_UTF8("u3"), BCON_UTF8("u4"), BCON_UTF8("u5"), "]", "}");
    mongoc_collection_delete_many(collection, del_filter, NULL, NULL, NULL);
    bson_destroy(del_filter);

    // Test case 1: Create
    printf("Create u1: %s\n", create_document(collection, "u1", "Alice", 30) ? "true" : "false");

    // Test case 2: Read
    char* json = read_document(collection, "u1");
    if (json) {
        printf("Read u1: %s\n", json);
        bson_free(json);
    } else {
        printf("Read u1: null\n");
    }

    // Test case 3: Update
    printf("Update u1: %s\n", update_document(collection, "u1", "Alicia", 31) ? "true" : "false");
    char* json2 = read_document(collection, "u1");
    if (json2) {
        printf("Read u1 after update: %s\n", json2);
        bson_free(json2);
    } else {
        printf("Read u1 after update: null\n");
    }

    // Test case 4: Create another
    printf("Create u2: %s\n", create_document(collection, "u2", "Bob", 25) ? "true" : "false");

    // Test case 5: Delete
    printf("Delete u2: %s\n", delete_document(collection, "u2") ? "true" : "false");

    mongoc_collection_destroy(collection);
    mongoc_database_destroy(db);
    mongoc_client_destroy(client);
    mongoc_uri_destroy(uri);
    mongoc_cleanup();
    return 0;
}