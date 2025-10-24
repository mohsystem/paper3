#include <stdio.h>
#include <stdlib.h>
#include <mongoc/mongoc.h>
#include <bson/bson.h>

/**
 * Note: To compile this code, you need the MongoDB C driver (libmongoc).
 * Installation instructions: http://mongoc.org/libmongoc/current/installing.html
 * 
 * Example compile command on Linux:
 * gcc -o task112_c task112.c $(pkg-config --cflags --libs libmongoc-1.0)
 * 
 * You must be very careful with memory management in C.
 * Always destroy BSON objects, cursors, and clients when you are done with them.
 */

// IMPORTANT: In a production environment, connection strings should never be hard-coded.
// They should be retrieved from a secure source like environment variables or a secrets manager.
const char *uri_string = "mongodb://localhost:27017";
const char *db_name = "mySecureDb";
const char *collection_name = "users";

// Function to create a document
// Returns true on success, false on failure. The oid is an out-parameter.
bool create_document(mongoc_collection_t *collection, const char *name, int age, bson_oid_t *oid) {
    bson_t *doc = NULL;
    bson_error_t error;
    bool success;

    doc = BCON_NEW("name", BCON_UTF8(name), "age", BCON_INT32(age));
    bson_oid_init(oid, NULL);
    BSON_APPEND_OID(doc, "_id", oid);

    success = mongoc_collection_insert_one(collection, doc, NULL, NULL, &error);

    if (success) {
        char oid_str[25];
        bson_oid_to_string(oid, oid_str);
        printf("Create: Success. Inserted document with id: %s\n", oid_str);
    } else {
        fprintf(stderr, "An error occurred during create operation: %s\n", error.message);
    }

    bson_destroy(doc);
    return success;
}

// Function to read a document by OID
void read_document(mongoc_collection_t *collection, const bson_oid_t *oid) {
    bson_t *query = NULL;
    const bson_t *doc;
    mongoc_cursor_t *cursor;
    char *str;

    query = BCON_NEW("_id", BCON_OID(oid, oid));
    cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);

    if (mongoc_cursor_next(cursor, &doc)) {
        str = bson_as_canonical_extended_json(doc, NULL);
        printf("Read: Success. Found document: %s\n", str);
        bson_free(str);
    } else {
        char oid_str[25];
        bson_oid_to_string(oid, oid_str);
        printf("Read: Document with id %s not found.\n", oid_str);
    }
    
    if (mongoc_cursor_error(cursor, NULL)) {
         fprintf(stderr, "An error occurred during read operation.\n");
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(query);
}

// Function to update a document
bool update_document(mongoc_collection_t *collection, const bson_oid_t *oid, int new_age) {
    bson_t *selector = NULL;
    bson_t *update = NULL;
    bson_error_t error;
    bool success;

    selector = BCON_NEW("_id", BCON_OID(oid, oid));
    update = BCON_NEW("$set", "{", "age", BCON_INT32(new_age), "}");
    
    success = mongoc_collection_update_one(collection, selector, update, NULL, NULL, &error);

    if (success) {
        printf("Update: Success. Update command sent.\n"); // C driver doesn't easily return matched/modified count here
    } else {
        fprintf(stderr, "An error occurred during update operation: %s\n", error.message);
    }
    
    bson_destroy(selector);
    bson_destroy(update);
    return success;
}

// Function to delete a document
bool delete_document(mongoc_collection_t *collection, const bson_oid_t *oid) {
    bson_t *selector = NULL;
    bson_error_t error;
    bool success;

    selector = BCON_NEW("_id", BCON_OID(oid, oid));
    success = mongoc_collection_delete_one(collection, selector, NULL, NULL, &error);

    if (success) {
        printf("Delete: Success. Delete command sent.\n");
    } else {
        fprintf(stderr, "An error occurred during delete operation: %s\n", error.message);
    }
    
    bson_destroy(selector);
    return success;
}

int main(void) {
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;
    bson_oid_t user_id;
    bson_error_t error;

    mongoc_init();

    client = mongoc_client_new(uri_string);
    if (!client) {
        fprintf(stderr, "Failed to create a new client instance\n");
        return EXIT_FAILURE;
    }

    database = mongoc_client_get_database(client, db_name);
    collection = mongoc_client_get_collection(client, db_name, collection_name);
    printf("Successfully connected to MongoDB.\n");

    // Cleanup before tests
    mongoc_collection_drop(collection, &error); // It's okay if this fails (e.g., collection doesn't exist)
    printf("--- Starting Test Cases ---\n");
    
    // --- Test Case 1: Create a new user ---
    printf("\n1. === CREATE Operation ===\n");
    if (create_document(collection, "Peter Parker", 22, &user_id)) {
        
        // --- Test Case 2: Read the created user ---
        printf("\n2. === READ Operation (after create) ===\n");
        read_document(collection, &user_id);
        
        // --- Test Case 3: Update the user's age ---
        printf("\n3. === UPDATE Operation ===\n");
        update_document(collection, &user_id, 23);
        
        // --- Test Case 4: Read the updated user ---
        printf("\n4. === READ Operation (after update) ===\n");
        read_document(collection, &user_id);
        
        // --- Test Case 5: Delete the user ---
        printf("\n5. === DELETE Operation ===\n");
        delete_document(collection, &user_id);
        
        // --- Final check: try to read the deleted user ---
        printf("\n--- Final Check: Verifying Deletion ---\n");
        read_document(collection, &user_id);

    } else {
        printf("Test cases aborted due to creation failure.\n");
    }

    printf("\n--- Test Cases Finished ---\n");

    // Clean up resources
    mongoc_collection_destroy(collection);
    mongoc_database_destroy(database);
    mongoc_client_destroy(client);
    mongoc_cleanup();

    return EXIT_SUCCESS;
}