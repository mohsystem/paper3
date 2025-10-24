#include <stdio.h>
#include <stdlib.h>
#include <mongoc/mongoc.h>
#include <bson/bson.h>

/*
NOTE: To compile this code, you need the MongoDB C driver (libmongoc).
Installation instructions: http://mongoc.org/libmongoc/current/installing.html

Example compilation command (after installing the driver):
gcc -o Task112 Task112.c $(pkg-config --cflags --libs libmongoc-1.0)
*/

// CREATE
void create_document(mongoc_collection_t *collection, const bson_t *doc) {
    bson_error_t error;
    if (!mongoc_collection_insert_one(collection, doc, NULL, NULL, &error)) {
        fprintf(stderr, "Error inserting document: %s\n", error.message);
    } else {
        printf("Success: Document inserted.\n");
    }
}

// READ
void read_document(mongoc_collection_t *collection, const bson_t *query) {
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    char *str;

    cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);

    if (mongoc_cursor_next(cursor, &doc)) {
        str = bson_as_canonical_extended_json(doc, NULL);
        printf("Found document: %s\n", str);
        bson_free(str);
    } else {
        printf("Document not found.\n");
    }

    mongoc_cursor_destroy(cursor);
}

// UPDATE
void update_document(mongoc_collection_t *collection, const bson_t *filter, const bson_t *update) {
    bson_error_t error;
    if (!mongoc_collection_update_one(collection, filter, update, NULL, NULL, &error)) {
        fprintf(stderr, "Error updating document: %s\n", error.message);
    } else {
        printf("Success: Update operation sent.\n");
    }
}

// DELETE
void delete_document(mongoc_collection_t *collection, const bson_t *filter) {
    bson_error_t error;
    if (!mongoc_collection_delete_one(collection, filter, NULL, NULL, &error)) {
        fprintf(stderr, "Error deleting document: %s\n", error.message);
    } else {
        printf("Success: Delete operation sent.\n");
    }
}

int main() {
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    bson_t *doc_to_create = NULL;
    bson_t *query = NULL;
    bson_t *update = NULL;
    bson_error_t error;

    // Required to initialize libmongoc's internals
    mongoc_init();

    // --- Connection Setup ---
    const char *uri_string = "mongodb://localhost:27017";
    client = mongoc_client_new(uri_string);

    if (!client) {
        fprintf(stderr, "Failed to create a new client.\n");
        return EXIT_FAILURE;
    }
    
    // Check connection
    if (!mongoc_client_command_simple(client, "admin", BCON_NEW("ping", BCON_INT32(1)), NULL, NULL, &error)) {
        fprintf(stderr, "Failed to connect to MongoDB: %s\n", error.message);
        mongoc_client_destroy(client);
        mongoc_cleanup();
        return EXIT_FAILURE;
    }
    printf("MongoDB connection successful.\n");

    const char *db_name = "cTestDB";
    const char *collection_name = "users";
    collection = mongoc_client_get_collection(client, db_name, collection_name);

    // Clean up previous runs
    mongoc_collection_drop(collection, NULL);
    printf("--- Starting CRUD Test Cases ---\n");

    // --- Test Case 1: Create ---
    printf("\n1. CREATE Operation\n");
    doc_to_create = BCON_NEW("name", BCON_UTF8("John von Neumann"),
                             "age", BCON_INT32(53),
                             "profession", BCON_UTF8("Mathematician"));
    create_document(collection, doc_to_create);

    // --- Test Case 2: Read ---
    printf("\n2. READ Operation\n");
    query = BCON_NEW("name", BCON_UTF8("John von Neumann"));
    read_document(collection, query);

    // --- Test Case 3: Update ---
    printf("\n3. UPDATE Operation\n");
    // The query is the same as the read query
    update = BCON_NEW("$set", "{", "age", BCON_INT32(54), "}");
    update_document(collection, query, update);

    // --- Test Case 4: Read after Update ---
    printf("\n4. READ Operation (after update)\n");
    read_document(collection, query);

    // --- Test Case 5: Delete ---
    printf("\n5. DELETE Operation\n");
    delete_document(collection, query);
    printf("Checking document after deletion attempt:\n");
    read_document(collection, query);

    printf("\n--- CRUD Test Cases Finished ---\n");

    // --- Final Cleanup ---
    printf("Dropping collection...\n");
    if (mongoc_collection_drop(collection, &error)) {
        printf("Collection dropped.\n");
    } else {
        fprintf(stderr, "Error dropping collection: %s\n", error.message);
    }
    
    bson_destroy(doc_to_create);
    bson_destroy(query);
    bson_destroy(update);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);
    mongoc_cleanup();

    return EXIT_SUCCESS;
}