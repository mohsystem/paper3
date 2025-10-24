/*
 * PRE-REQUISITES:
 * 1. Install the MongoDB C driver (libmongoc). Follow instructions at:
 *    http://mongoc.org/libmongoc/current/installing.html
 *
 * COMPILE & RUN (example on Linux/macOS):
 * gcc Task112.c -o Task112 $(pkg-config --cflags --libs libmongoc-1.0)
 *
 * ./Task112
 */
#include <stdio.h>
#include <stdlib.h>
#include <mongoc/mongoc.h>
#include <bson/bson.h>

// Returns true on success, false on failure
bool createUser(mongoc_collection_t *collection, const bson_t *doc, bson_oid_t *oid) {
    bson_error_t error;
    bool success = mongoc_collection_insert_one(collection, doc, NULL, NULL, &error);
    if (!success) {
        fprintf(stderr, "Error creating user: %s\n", error.message);
        return false;
    }
    bson_iter_t iter;
    if (bson_iter_init_find(&iter, doc, "_id")) {
        bson_oid_copy(bson_iter_oid(&iter), oid);
    }
    return true;
}

// Returns a new bson_t document that must be destroyed by the caller, or NULL on failure/not found
bson_t* readUser(mongoc_collection_t *collection, const bson_t *filter) {
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(collection, filter, NULL, NULL);
    const bson_t *doc;
    bson_t* result = NULL;
    if (mongoc_cursor_next(cursor, &doc)) {
        result = bson_copy(doc);
    }
    if(mongoc_cursor_error(cursor, NULL)){
        // Error handling
    }
    mongoc_cursor_destroy(cursor);
    return result;
}

// Returns number of modified documents, or -1 on failure
int64_t updateUser(mongoc_collection_t *collection, const bson_t *filter, const bson_t *update) {
    bson_error_t error;
    bson_t reply;
    bool success = mongoc_collection_update_one(collection, filter, update, NULL, &reply, &error);
    if (!success) {
        fprintf(stderr, "Error updating user: %s\n", error.message);
        bson_destroy(&reply);
        return -1;
    }
    
    int64_t modified_count = 0;
    bson_iter_t iter;
    if (bson_iter_init_find(&iter, &reply, "modifiedCount")) {
        modified_count = bson_iter_int64(&iter);
    }

    bson_destroy(&reply);
    return modified_count;
}

// Returns number of deleted documents, or -1 on failure
int64_t deleteUser(mongoc_collection_t *collection, const bson_t *filter) {
    bson_error_t error;
    bson_t reply;
    bool success = mongoc_collection_delete_one(collection, filter, NULL, &reply, &error);
    if (!success) {
        fprintf(stderr, "Error deleting user: %s\n", error.message);
        bson_destroy(&reply);
        return -1;
    }

    int64_t deleted_count = 0;
    bson_iter_t iter;
    if (bson_iter_init_find(&iter, &reply, "deletedCount")) {
        deleted_count = bson_iter_int64(&iter);
    }

    bson_destroy(&reply);
    return deleted_count;
}

int main(void) {
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;
    bson_error_t error;
    bson_oid_t oid;
    bson_t *doc = NULL;
    bson_t *read_filter = NULL;

    mongoc_init();

    const char *uri_string = "mongodb://localhost:27017";
    client = mongoc_client_new(uri_string);
    if (!client) {
        fprintf(stderr, "Failed to create a new client.\n");
        return EXIT_FAILURE;
    }
    
    database = mongoc_client_get_database(client, "c_test_db");
    collection = mongoc_client_get_collection(client, "c_test_db", "users");
    
    // Clean up collection before tests
    mongoc_collection_drop(collection, &error); // Ignore error if collection doesn't exist
    
    printf("--- Starting MongoDB CRUD Test Cases ---\n");

    // Test Case 1: Create
    printf("\n--- Test Case 1: Create User ---\n");
    doc = BCON_NEW("name", BCON_UTF8("Susan Smith"), "age", BCON_INT32(25), "city", BCON_UTF8("Toronto"));
    // The createUser function needs the OID, so we add it first.
    bson_oid_init(&oid, NULL);
    BSON_APPEND_OID(doc, "_id", &oid);

    if (createUser(collection, doc, &oid)) {
        char oid_str[25];
        bson_oid_to_string(&oid, oid_str);
        printf("User created successfully with ID: %s\n", oid_str);
    } else {
        printf("User creation failed.\n");
    }
    bson_destroy(doc);
    
    // Test Case 2: Read
    printf("\n--- Test Case 2: Read User ---\n");
    read_filter = BCON_NEW("_id", BCON_OID(&oid));
    bson_t* found_user = readUser(collection, read_filter);
    if (found_user) {
        char *str = bson_as_canonical_extended_json(found_user, NULL);
        printf("User found: %s\n", str);
        bson_free(str);
        bson_destroy(found_user);
    } else {
        printf("User not found.\n");
    }

    // Test Case 3: Update
    printf("\n--- Test Case 3: Update User ---\n");
    bson_t *update = BCON_NEW("$set", "{", "age", BCON_INT32(26), "}");
    int64_t modified_count = updateUser(collection, read_filter, update);
    if(modified_count >= 0) {
        printf("Users updated: %lld\n", (long long)modified_count);
    }
    bson_destroy(update);

    // Test Case 4: Read after Update
    printf("\n--- Test Case 4: Read User After Update ---\n");
    bson_t* updated_user = readUser(collection, read_filter);
    if (updated_user) {
        char *str = bson_as_canonical_extended_json(updated_user, NULL);
        printf("User found after update: %s\n", str);
        bson_free(str);
        bson_destroy(updated_user);
    } else {
        printf("User not found.\n");
    }

    // Test Case 5: Delete
    printf("\n--- Test Case 5: Delete User ---\n");
    int64_t deleted_count = deleteUser(collection, read_filter);
    if(deleted_count >= 0) {
        printf("Users deleted: %lld\n", (long long)deleted_count);
    }
    
    bson_t* deleted_user = readUser(collection, read_filter);
    if (deleted_user == NULL) {
        printf("User successfully deleted and not found.\n");
    } else {
        printf("User deletion failed.\n");
        bson_destroy(deleted_user);
    }

    bson_destroy(read_filter);

    // Cleanup
    mongoc_collection_destroy(collection);
    mongoc_database_destroy(database);
    mongoc_client_destroy(client);
    mongoc_cleanup();

    printf("\n--- Tests Finished ---\n");
    return EXIT_SUCCESS;
}