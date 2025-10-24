/*
 * To compile and run this code, you need the MongoDB C driver (mongo-c-driver).
 *
 * Example compilation command using pkg-config:
 * gcc -o task112_c task112.c $(pkg-config --cflags --libs libmongoc)
 *
 * Make sure you have a MongoDB instance running on localhost:27017.
 */
#include <mongoc/mongoc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// NOTE: For a production environment, the connection string should not be hardcoded.
// It should be loaded from a secure configuration source or environment variables.
const char *URI = "mongodb://localhost:27017";
const char *DATABASE_NAME = "my_c_db";
const char *COLLECTION_NAME = "users";

/**
 * Creates a new document in the specified collection.
 * @param collection The MongoDB collection.
 * @param name The name of the user.
 * @param email The email of the user.
 * @return true on success, false on failure.
 */
bool createUser(mongoc_collection_t *collection, const char *name, const char *email) {
    if (!name || !email || strlen(name) == 0 || strlen(email) == 0) {
        fprintf(stderr, "Error: Name and email cannot be NULL or empty.\n");
        return false;
    }
    bson_t *doc;
    bson_error_t error;
    bool success;

    doc = BCON_NEW("name", BCON_UTF8(name), "email", BCON_UTF8(email));
    success = mongoc_collection_insert_one(collection, doc, NULL, NULL, &error);

    if (!success) {
        fprintf(stderr, "Error creating user: %s\n", error.message);
    }
    
    bson_destroy(doc);
    return success;
}

/**
 * Reads a document from the collection by name and returns it as a JSON string.
 * @param collection The MongoDB collection.
 * @param name The name of the user to find.
 * @return A heap-allocated JSON string (must be freed by caller), or NULL if not found or on error.
 */
char* readUserByName(mongoc_collection_t *collection, const char *name) {
    if (!name || strlen(name) == 0) {
        fprintf(stderr, "Error: Name cannot be NULL or empty.\n");
        return NULL;
    }
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    bson_t *query;
    char *json_str = NULL;
    
    query = BCON_NEW("name", BCON_UTF8(name));
    cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);

    if (mongoc_cursor_next(cursor, &doc)) {
        json_str = bson_as_canonical_extended_json(doc, NULL);
    }

    bson_error_t error;
    if (mongoc_cursor_error(cursor, &error)) {
        fprintf(stderr, "Cursor error: %s\n", error.message);
        if (json_str) {
            bson_free(json_str);
            json_str = NULL;
        }
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(query);
    return json_str;
}

/**
 * Updates a user's email by their name.
 * @param collection The MongoDB collection.
 * @param name The name of the user to update.
 * @param newEmail The new email address.
 * @return true if the update was successful, false otherwise.
 */
bool updateUserEmail(mongoc_collection_t *collection, const char *name, const char *newEmail) {
    if (!name || !newEmail || strlen(name) == 0 || strlen(newEmail) == 0) {
        fprintf(stderr, "Error: Name and new email cannot be NULL or empty.\n");
        return false;
    }
    bson_t *selector, *update;
    bson_error_t error;
    bool success;
    
    selector = BCON_NEW("name", BCON_UTF8(name));
    update = BCON_NEW("$set", "{", "email", BCON_UTF8(newEmail), "}");

    success = mongoc_collection_update_one(collection, selector, update, NULL, NULL, &error);

    if (!success) {
        fprintf(stderr, "Error updating user: %s\n", error.message);
    }
    
    bson_destroy(selector);
    bson_destroy(update);
    return success;
}

/**
 * Deletes a user by their name.
 * @param collection The MongoDB collection.
 * @param name The name of the user to delete.
 * @return true if the deletion was successful, false otherwise.
 */
bool deleteUser(mongoc_collection_t *collection, const char *name) {
    if (!name || strlen(name) == 0) {
        fprintf(stderr, "Error: Name cannot be NULL or empty.\n");
        return false;
    }
    bson_t *selector;
    bson_error_t error;
    bool success;
    
    selector = BCON_NEW("name", BCON_UTF8(name));
    success = mongoc_collection_delete_one(collection, selector, NULL, NULL, &error);

    if (!success) {
        fprintf(stderr, "Error deleting user: %s\n", error.message);
    }
    
    bson_destroy(selector);
    return success;
}

int main() {
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;
    bson_error_t error;

    mongoc_init();

    client = mongoc_client_new(URI);
    if (!client) {
        fprintf(stderr, "Failed to create a new client instance\n");
        mongoc_cleanup();
        return EXIT_FAILURE;
    }

    database = mongoc_client_get_database(client, DATABASE_NAME);
    collection = mongoc_client_get_collection(client, DATABASE_NAME, COLLECTION_NAME);
    
    // Clean up the collection before running tests
    mongoc_collection_drop(collection, &error); // Don't need to check error, collection might not exist
    printf("Collection dropped for a clean test run.\n");
    
    printf("--- MongoDB CRUD Operations in C ---\n");
    
    // 1. Create Operation
    printf("\n[Test Case 1: Create]\n");
    if (createUser(collection, "C User", "c.user@example.com")) {
        printf("User created successfully.\n");
    } else {
        printf("User creation failed.\n");
    }

    // 2. Read Operation
    printf("\n[Test Case 2: Read]\n");
    char* user_json = readUserByName(collection, "C User");
    if (user_json) {
        printf("User found: %s\n", user_json);
        bson_free(user_json);
    } else {
        printf("User 'C User' not found.\n");
    }

    // 3. Update Operation
    printf("\n[Test Case 3: Update]\n");
    if (updateUserEmail(collection, "C User", "c.new@example.com")) {
        printf("User 'C User' updated successfully.\n");
    } else {
        printf("User update failed.\n");
    }

    // 4. Read again to verify update
    printf("\n[Test Case 4: Verify Update]\n");
    user_json = readUserByName(collection, "C User");
    if (user_json) {
        printf("User found after update: %s\n", user_json);
        bson_free(user_json);
    } else {
        printf("User 'C User' not found after update.\n");
    }

    // 5. Delete Operation
    printf("\n[Test Case 5: Delete]\n");
    if (deleteUser(collection, "C User")) {
        printf("User 'C User' deleted successfully.\n");
    } else {
        printf("User deletion failed.\n");
    }

    // Verify deletion
    user_json = readUserByName(collection, "C User");
    if (!user_json) {
        printf("Successfully verified that 'C User' is no longer in the database.\n");
    } else {
        printf("Verification failed: 'C User' still exists.\n");
        bson_free(user_json);
    }

    // Cleanup
    mongoc_collection_destroy(collection);
    mongoc_database_destroy(database);
    mongoc_client_destroy(client);
    mongoc_cleanup();

    return EXIT_SUCCESS;
}