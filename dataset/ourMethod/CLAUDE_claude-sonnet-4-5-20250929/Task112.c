
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mongoc/mongoc.h>

#define MAX_STRING_LEN 1000
#define MAX_RESULTS 100

/* Security: Validate string length to prevent buffer overflow (CWE-120, CWE-787) */
int is_valid_string_length(const char* str, size_t max_len) {
    if (!str) return 0;
    size_t len = strnlen(str, max_len + 1);
    return len > 0 && len <= max_len;
}

/* Security: Validate input parameters before database operations (CWE-20) */
int validate_input(const char* input) {
    if (!input) return 0;
    
    if (!is_valid_string_length(input, MAX_STRING_LEN)) {
        return 0;
    }
    
    /* Security: Check for null bytes that could truncate strings (CWE-170) */
    size_t len = strnlen(input, MAX_STRING_LEN + 1);
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\\0' && i < len - 1) {
            return 0;
        }
    }
    
    return 1;
}

typedef struct {
    mongoc_client_t* client;
    mongoc_database_t* database;
    mongoc_collection_t* collection;
    int initialized;
} MongoDBClient;

/* Security: Initialize all pointers to NULL (C Security Checklist) */
void init_client(MongoDBClient* client) {
    if (!client) return;
    client->client = NULL;
    client->database = NULL;
    client->collection = NULL;
    client->initialized = 0;
}

/* Security: Use environment variable for connection string (CWE-798) */
int connect_db(MongoDBClient* client, const char* db_name, const char* coll_name) {
    if (!client || !db_name || !coll_name) {
        fprintf(stderr, "Invalid parameters\\n");
        return 0;
    }

    /* Security: Validate all inputs before use (CWE-20) */
    if (!validate_input(db_name) || !validate_input(coll_name)) {
        fprintf(stderr, "Invalid database or collection name\\n");
        return 0;
    }

    const char* uri_string = getenv("MONGODB_URI");
    if (!uri_string) {
        uri_string = "mongodb://localhost:27017";
    }

    mongoc_init();
    
    /* Security: Create client with error handling (CWE-755) */
    bson_error_t error;
    mongoc_uri_t* uri = mongoc_uri_new_with_error(uri_string, &error);
    if (!uri) {
        fprintf(stderr, "Failed to parse URI: %s\\n", error.message);
        return 0;
    }

    client->client = mongoc_client_new_from_uri(uri);
    mongoc_uri_destroy(uri);
    
    /* Security: Check malloc/allocation return values (C Security Checklist) */
    if (!client->client) {
        fprintf(stderr, "Failed to create client\\n");
        return 0;
    }

    client->database = mongoc_client_get_database(client->client, db_name);
    client->collection = mongoc_client_get_collection(client->client, db_name, coll_name);
    
    if (!client->database || !client->collection) {
        fprintf(stderr, "Failed to get database or collection\\n");
        return 0;
    }
    
    client->initialized = 1;
    return 1;
}

/* Create operation - Security: Validate all inputs (CWE-20) */
int create_document(MongoDBClient* client, const char* name, int age) {
    if (!client || !client->initialized) {
        fprintf(stderr, "Client not initialized\\n");
        return 0;
    }

    /* Security: Validate input (CWE-20) */
    if (!validate_input(name)) {
        fprintf(stderr, "Invalid name\\n");
        return 0;
    }

    /* Security: Check for integer overflow and valid range (CWE-190) */
    if (age < 0 || age > 150) {
        fprintf(stderr, "Invalid age\\n");
        return 0;
    }

    bson_error_t error;
    bson_t* doc = bson_new();
    
    if (!doc) {
        fprintf(stderr, "Failed to create BSON document\\n");
        return 0;
    }
    
    /* Security: Use BSON API properly to prevent injection (CWE-943) */
    BSON_APPEND_UTF8(doc, "name", name);
    BSON_APPEND_INT32(doc, "age", age);

    int success = mongoc_collection_insert_one(client->collection, doc, NULL, NULL, &error);
    
    if (!success) {
        fprintf(stderr, "Insert failed: %s\\n", error.message);
    }

    bson_destroy(doc);
    return success;
}

/* Read operation - Security: Limit result size to prevent DoS (CWE-400) */
int read_documents(MongoDBClient* client, const char* name, char*** results, int* count) {
    if (!client || !client->initialized || !results || !count) {
        fprintf(stderr, "Invalid parameters\\n");
        return 0;
    }

    /* Security: Initialize output parameters */
    *results = NULL;
    *count = 0;

    if (!validate_input(name)) {
        fprintf(stderr, "Invalid name\\n");
        return 0;
    }

    bson_t* query = bson_new();
    if (!query) {
        fprintf(stderr, "Failed to create query\\n");
        return 0;
    }
    
    BSON_APPEND_UTF8(query, "name", name);

    mongoc_cursor_t* cursor = mongoc_collection_find_with_opts(client->collection, query, NULL, NULL);
    
    if (!cursor) {
        fprintf(stderr, "Failed to create cursor\\n");
        bson_destroy(query);
        return 0;
    }
    
    const bson_t* doc;
    int result_count = 0;
    
    /* Security: Allocate initial array with bounds check */
    char** result_array = (char**)calloc(MAX_RESULTS, sizeof(char*));
    if (!result_array) {
        fprintf(stderr, "Memory allocation failed\\n");
        mongoc_cursor_destroy(cursor);
        bson_destroy(query);
        return 0;
    }

    /* Security: Limit results to prevent resource exhaustion (CWE-400) */
    while (mongoc_cursor_next(cursor, &doc) && result_count < MAX_RESULTS) {
        size_t len = 0;
        char* str = bson_as_canonical_extended_json(doc, &len);
        
        if (str && len > 0) {
            /* Security: Allocate exact size needed plus null terminator */
            result_array[result_count] = (char*)malloc(len + 1);
            if (result_array[result_count]) {
                /* Security: Use bounds-checked copy */
                strncpy(result_array[result_count], str, len);
                result_array[result_count][len] = '\\0';
                result_count++;
            }
            bson_free(str);
        }
    }

    /* Security: Check for cursor errors (CWE-755) */
    bson_error_t error;
    if (mongoc_cursor_error(cursor, &error)) {
        fprintf(stderr, "Cursor error: %s\\n", error.message);
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(query);
    
    *results = result_array;
    *count = result_count;
    
    return 1;
}

/* Update operation - Security: Validate inputs */
int update_document(MongoDBClient* client, const char* name, int new_age) {
    if (!client || !client->initialized) {
        fprintf(stderr, "Client not initialized\\n");
        return 0;
    }

    if (!validate_input(name)) {
        fprintf(stderr, "Invalid name\\n");
        return 0;
    }

    /* Security: Validate age range (CWE-190) */
    if (new_age < 0 || new_age > 150) {
        fprintf(stderr, "Invalid age\\n");
        return 0;
    }

    bson_t* query = bson_new();
    if (!query) {
        fprintf(stderr, "Failed to create query\\n");
        return 0;
    }
    
    BSON_APPEND_UTF8(query, "name", name);

    bson_t* update = bson_new();
    if (!update) {
        fprintf(stderr, "Failed to create update document\\n");
        bson_destroy(query);
        return 0;
    }
    
    bson_t child;
    BSON_APPEND_DOCUMENT_BEGIN(update, "$set", &child);
    BSON_APPEND_INT32(&child, "age", new_age);
    bson_append_document_end(update, &child);

    bson_error_t error;
    int success = mongoc_collection_update_one(client->collection, query, update, NULL, NULL, &error);

    if (!success) {
        fprintf(stderr, "Update failed: %s\\n", error.message);
    }

    bson_destroy(query);
    bson_destroy(update);
    
    return success;
}

/* Delete operation - Security: Validate input */
int delete_document(MongoDBClient* client, const char* name) {
    if (!client || !client->initialized) {
        fprintf(stderr, "Client not initialized\\n");
        return 0;
    }

    if (!validate_input(name)) {
        fprintf(stderr, "Invalid name\\n");
        return 0;
    }

    bson_t* query = bson_new();
    if (!query) {
        fprintf(stderr, "Failed to create query\\n");
        return 0;
    }
    
    BSON_APPEND_UTF8(query, "name", name);

    bson_error_t error;
    int success = mongoc_collection_delete_one(client->collection, query, NULL, NULL, &error);

    if (!success) {
        fprintf(stderr, "Delete failed: %s\\n", error.message);
    }

    bson_destroy(query);
    return success;
}

/* Security: Free all allocated memory (C Security Checklist) */
void cleanup_results(char** results, int count) {
    if (!results) return;
    
    for (int i = 0; i < count; i++) {
        if (results[i]) {
            free(results[i]);
            results[i] = NULL;
        }
    }
    free(results);
}

/* Security: RAII-style cleanup (CWE-404) */
void disconnect_db(MongoDBClient* client) {
    if (!client) return;
    
    if (client->collection) {
        mongoc_collection_destroy(client->collection);
        client->collection = NULL;
    }
    if (client->database) {
        mongoc_database_destroy(client->database);
        client->database = NULL;
    }
    if (client->client) {
        mongoc_client_destroy(client->client);
        client->client = NULL;
    }
    if (client->initialized) {
        mongoc_cleanup();
        client->initialized = 0;
    }
}

int main(void) {
    MongoDBClient client;
    init_client(&client);
    
    if (!connect_db(&client, "testdb", "users")) {
        fprintf(stderr, "Failed to connect to MongoDB\\n");
        return 1;
    }

    printf("Test 1: Create document\\n");
    if (create_document(&client, "Alice", 30)) {
        printf("Created document for Alice\\n");
    }

    printf("\\nTest 2: Create another document\\n");
    if (create_document(&client, "Bob", 25)) {
        printf("Created document for Bob\\n");
    }

    printf("\\nTest 3: Read documents\\n");
    char** results = NULL;
    int count = 0;
    if (read_documents(&client, "Alice", &results, &count)) {
        for (int i = 0; i < count; i++) {
            printf("Found: %s\\n", results[i]);
        }
        cleanup_results(results, count);
    }

    printf("\\nTest 4: Update document\\n");
    if (update_document(&client, "Alice", 31)) {
        printf("Updated Alice's age\\n");
    }
    
    results = NULL;
    count = 0;
    if (read_documents(&client, "Alice", &results, &count)) {
        for (int i = 0; i < count; i++) {
            printf("After update: %s\\n", results[i]);
        }
        cleanup_results(results, count);
    }

    printf("\\nTest 5: Delete document\\n");
    if (delete_document(&client, "Bob")) {
        printf("Deleted Bob's document\\n");
    }

    disconnect_db(&client);
    return 0;
}
