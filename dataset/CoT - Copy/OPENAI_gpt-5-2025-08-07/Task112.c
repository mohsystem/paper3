#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <bson/bson.h>
#include <mongoc/mongoc.h>

// Create MongoDB client securely using env var and timeouts via URI
mongoc_client_t* create_client(void) {
    const char* env_uri = getenv("MONGODB_URI");
    const char* default_uri = "mongodb://localhost:27017/?retryWrites=true&w=majority&serverSelectionTimeoutMS=10000&connectTimeoutMS=10000";
    const char* uri_str = (env_uri && env_uri[0] != '\0') ? env_uri : default_uri;
    mongoc_uri_t* uri = mongoc_uri_new(uri_str);
    if (!uri) {
        fprintf(stderr, "Invalid MongoDB URI\n");
        return NULL;
    }
    mongoc_client_t* client = mongoc_client_new_from_uri(uri);
    mongoc_uri_destroy(uri);
    if (!client) {
        fprintf(stderr, "Failed to create MongoDB client\n");
        return NULL;
    }
    mongoc_client_set_appname(client, "Task112CClient");
    return client;
}

mongoc_collection_t* get_collection(mongoc_client_t* client, const char* db_name, const char* coll_name) {
    if (!client || !db_name || !coll_name) return NULL;
    mongoc_database_t* db = mongoc_client_get_database(client, db_name);

    // Set read concern majority
    mongoc_read_concern_t* rc = mongoc_read_concern_new();
    mongoc_read_concern_set_level(rc, MONGOC_READ_CONCERN_LEVEL_MAJORITY);
    mongoc_database_set_read_concern(db, rc);
    mongoc_read_concern_destroy(rc);

    // Get collection and set write concern majority
    mongoc_collection_t* coll = mongoc_client_get_collection(client, db_name, coll_name);
    mongoc_write_concern_t* wc = mongoc_write_concern_new();
    mongoc_write_concern_set_wmajority(wc, 5000 /* wtimeout ms */);
    mongoc_collection_set_write_concern(coll, wc);
    mongoc_write_concern_destroy(wc);

    mongoc_database_destroy(db);
    return coll;
}

// Create document from JSON string; returns true if success and sets out_oid
bool create_document(mongoc_collection_t* coll, const char* json_doc, bson_oid_t* out_oid) {
    if (!coll || !json_doc) return false;
    bson_error_t error;
    bson_t* doc = bson_new_from_json((const uint8_t*)json_doc, -1, &error);
    if (!doc) {
        fprintf(stderr, "JSON parse error: %s\n", error.message);
        return false;
    }
    // Ensure _id exists to retrieve later
    bson_oid_t oid_local;
    if (!bson_has_field(doc, "_id")) {
        bson_oid_init(&oid_local, NULL);
        BSON_APPEND_OID(doc, "_id", &oid_local);
        if (out_oid) *out_oid = oid_local;
    } else if (out_oid) {
        bson_iter_t iter;
        if (bson_iter_init_find(&iter, doc, "_id") && BSON_ITER_HOLDS_OID(&iter)) {
            const bson_oid_t* id = bson_iter_oid(&iter);
            *out_oid = *id;
        }
    }

    bson_t reply;
    bool ok = mongoc_collection_insert_one(coll, doc, NULL, &reply, &error);
    if (!ok) {
        fprintf(stderr, "Insert error: %s\n", error.message);
    }
    bson_destroy(&reply);
    bson_destroy(doc);
    return ok;
}

// Read documents matching filter JSON; returns count of matched docs
int read_documents(mongoc_collection_t* coll, const char* json_filter) {
    if (!coll) return -1;
    bson_error_t error;
    bson_t* filter = NULL;
    if (json_filter && json_filter[0]) {
        filter = bson_new_from_json((const uint8_t*)json_filter, -1, &error);
        if (!filter) {
            fprintf(stderr, "Filter JSON parse error: %s\n", error.message);
            return -1;
        }
    } else {
        filter = bson_new();
    }

    mongoc_cursor_t* cursor = mongoc_collection_find_with_opts(coll, filter, NULL, NULL);
    const bson_t* doc;
    int count = 0;
    while (mongoc_cursor_next(cursor, &doc)) {
        count++;
    }
    if (mongoc_cursor_error(cursor, &error)) {
        fprintf(stderr, "Find error: %s\n", error.message);
        count = -1;
    }
    mongoc_cursor_destroy(cursor);
    bson_destroy(filter);
    return count;
}

// Update documents matching filter JSON with update JSON; returns modified count
long long update_documents(mongoc_collection_t* coll, const char* json_filter, const char* json_update) {
    if (!coll || !json_filter || !json_update) return -1;
    bson_error_t error;
    bson_t* filter = bson_new_from_json((const uint8_t*)json_filter, -1, &error);
    if (!filter) {
        fprintf(stderr, "Filter JSON parse error: %s\n", error.message);
        return -1;
    }
    bson_t* update = bson_new_from_json((const uint8_t*)json_update, -1, &error);
    if (!update) {
        fprintf(stderr, "Update JSON parse error: %s\n", error.message);
        bson_destroy(filter);
        return -1;
    }

    bson_t reply;
    bool ok = mongoc_collection_update_many(coll, filter, update, NULL, &reply, &error);
    long long modified = 0;
    if (!ok) {
        fprintf(stderr, "Update error: %s\n", error.message);
    } else {
        bson_iter_t it;
        if (bson_iter_init_find(&it, &reply, "modifiedCount") && BSON_ITER_HOLDS_INT64(&it)) {
            modified = bson_iter_int64(&it);
        }
    }
    bson_destroy(&reply);
    bson_destroy(filter);
    bson_destroy(update);
    return modified;
}

// Delete documents matching filter JSON; returns deleted count
long long delete_documents(mongoc_collection_t* coll, const char* json_filter) {
    if (!coll || !json_filter) return -1;
    bson_error_t error;
    bson_t* filter = bson_new_from_json((const uint8_t*)json_filter, -1, &error);
    if (!filter) {
        fprintf(stderr, "Filter JSON parse error: %s\n", error.message);
        return -1;
    }
    bson_t reply;
    bool ok = mongoc_collection_delete_many(coll, filter, NULL, &reply, &error);
    long long deleted = 0;
    if (!ok) {
        fprintf(stderr, "Delete error: %s\n", error.message);
    } else {
        bson_iter_t it;
        if (bson_iter_init_find(&it, &reply, "deletedCount") && BSON_ITER_HOLDS_INT64(&it)) {
            deleted = bson_iter_int64(&it);
        }
    }
    bson_destroy(&reply);
    bson_destroy(filter);
    return deleted;
}

int main(void) {
    mongoc_init();
    const char* db_name = "task112db";
    const char* coll_name = "users";
    const char* tag = "task112";

    mongoc_client_t* client = create_client();
    if (!client) {
        mongoc_cleanup();
        return 1;
    }
    mongoc_collection_t* coll = get_collection(client, db_name, coll_name);
    if (!coll) {
        mongoc_client_destroy(client);
        mongoc_cleanup();
        return 1;
    }

    // Test 1: Cleanup
    char filter_cleanup[64];
    bson_snprintf(filter_cleanup, sizeof(filter_cleanup), "{ \"tag\": \"%s\" }", tag);
    long long cleaned = delete_documents(coll, filter_cleanup);
    printf("Test1-Cleanup deleted: %lld\n", cleaned);

    // Test 2: Create
    char doc_json[256];
    bson_snprintf(doc_json, sizeof(doc_json),
                  "{ \"username\": \"alice\", \"email\": \"alice.task112@example.com\", \"age\": 30, \"tag\": \"%s\" }",
                  tag);
    bson_oid_t inserted_oid;
    bool inserted = create_document(coll, doc_json, &inserted_oid);
    if (inserted) {
        char oid_str[25];
        bson_oid_to_string(&inserted_oid, oid_str);
        printf("Test2-Create insertedId: %s\n", oid_str);
    } else {
        printf("Test2-Create failed\n");
    }

    // Test 3: Read
    char filter_read[128];
    bson_snprintf(filter_read, sizeof(filter_read),
                  "{ \"username\": \"alice\", \"tag\": \"%s\" }", tag);
    int read_count = read_documents(coll, filter_read);
    printf("Test3-Read count: %d\n", read_count);

    // Test 4: Update
    char update_json[] = "{ \"$set\": { \"age\": 31 } }";
    long long modified = update_documents(coll, filter_read, update_json);
    printf("Test4-Update modified: %lld\n", modified);

    // Test 5: Delete
    long long deleted = delete_documents(coll, filter_read);
    printf("Test5-Delete deleted: %lld\n", deleted);

    mongoc_collection_destroy(coll);
    mongoc_client_destroy(client);
    mongoc_cleanup();
    return 0;
}