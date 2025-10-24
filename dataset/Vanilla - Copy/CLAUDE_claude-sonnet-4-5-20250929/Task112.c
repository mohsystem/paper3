
/*
 * Note: MongoDB does not have an official C driver that is as straightforward as other languages.
 * The mongo-c-driver (libmongoc) is available but requires significant setup and compilation.
 * Below is a conceptual implementation using libmongoc.
 * 
 * To compile, you need to install mongo-c-driver and link against libmongoc and libbson:
 * gcc -o task112 task112.c $(pkg-config --cflags --libs libmongoc-1.0)
 */

#include <mongoc/mongoc.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;
} Task112;

Task112* Task112_init(const char* connection_string, const char* db_name, const char* collection_name) {
    Task112* task = (Task112*)malloc(sizeof(Task112));
    mongoc_init();
    
    task->client = mongoc_client_new(connection_string);
    task->database = mongoc_client_get_database(task->client, db_name);
    task->collection = mongoc_client_get_collection(task->client, db_name, collection_name);
    
    return task;
}

// Create operation
char* create(Task112* task, const char* name, int age, const char* email) {
    bson_t *doc = bson_new();
    bson_append_utf8(doc, "name", -1, name, -1);
    bson_append_int32(doc, "age", -1, age);
    bson_append_utf8(doc, "email", -1, email, -1);
    
    bson_error_t error;
    if (!mongoc_collection_insert_one(task->collection, doc, NULL, NULL, &error)) {
        bson_destroy(doc);
        return "Failed to create document";
    }
    
    bson_destroy(doc);
    static char result[100];
    snprintf(result, sizeof(result), "Created document successfully");
    return result;
}

// Read operation
char* read(Task112* task, const char* name) {
    bson_t *query = bson_new();
    bson_append_utf8(query, "name", -1, name, -1);
    
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(task->collection, query, NULL, NULL);
    const bson_t *doc;
    static char result[1000];
    
    if (mongoc_cursor_next(cursor, &doc)) {
        char *str = bson_as_json(doc, NULL);
        snprintf(result, sizeof(result), "Found: %s", str);
        bson_free(str);
    } else {
        snprintf(result, sizeof(result), "Document not found");
    }
    
    mongoc_cursor_destroy(cursor);
    bson_destroy(query);
    return result;
}

// Update operation
char* update(Task112* task, const char* name, int new_age) {
    bson_t *query = bson_new();
    bson_append_utf8(query, "name", -1, name, -1);
    
    bson_t *update = BCON_NEW("$set", "{", "age", BCON_INT32(new_age), "}");
    
    bson_error_t error;
    if (!mongoc_collection_update_one(task->collection, query, update, NULL, NULL, &error)) {
        bson_destroy(query);
        bson_destroy(update);
        return "Update failed";
    }
    
    bson_destroy(query);
    bson_destroy(update);
    static char result[100];
    snprintf(result, sizeof(result), "Document updated successfully");
    return result;
}

// Delete operation
char* deleteDoc(Task112* task, const char* name) {
    bson_t *query = bson_new();
    bson_append_utf8(query, "name", -1, name, -1);
    
    bson_error_t error;
    if (!mongoc_collection_delete_one(task->collection, query, NULL, NULL, &error)) {
        bson_destroy(query);
        return "Delete failed";
    }
    
    bson_destroy(query);
    static char result[100];
    snprintf(result, sizeof(result), "Document deleted successfully");
    return result;
}

void Task112_cleanup(Task112* task) {
    mongoc_collection_destroy(task->collection);
    mongoc_database_destroy(task->database);
    mongoc_client_destroy(task->client);
    mongoc_cleanup();
    free(task);
}

int main() {
    const char* connection_string = "mongodb://localhost:27017";
    const char* db_name = "testdb";
    const char* collection_name = "users";
    
    Task112* mongoOps = Task112_init(connection_string, db_name, collection_name);
    
    printf("Test Case 1: Create a document\\n");
    printf("%s\\n", create(mongoOps, "John Doe", 30, "john@example.com"));
    
    printf("\\nTest Case 2: Read a document\\n");
    printf("%s\\n", read(mongoOps, "John Doe"));
    
    printf("\\nTest Case 3: Update a document\\n");
    printf("%s\\n", update(mongoOps, "John Doe", 31));
    
    printf("\\nTest Case 4: Read updated document\\n");
    printf("%s\\n", read(mongoOps, "John Doe"));
    
    printf("\\nTest Case 5: Delete a document\\n");
    printf("%s\\n", deleteDoc(mongoOps, "John Doe"));
    
    Task112_cleanup(mongoOps);
    return 0;
}
