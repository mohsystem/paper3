
/*
 * Note: MongoDB does not have an official C driver in the traditional sense.
 * The official MongoDB C driver is libmongoc, which requires external library installation.
 * This is a conceptual implementation showing the structure.
 * To compile and run, you need to install libmongoc and libbson libraries.
 * 
 * Installation: 
 * - Ubuntu/Debian: sudo apt-get install libmongoc-dev libbson-dev
 * - Compile: gcc -o task112 task112.c $(pkg-config --cflags --libs libmongoc-1.0)
 */

#include <stdio.h>
#include <string.h>
#include <mongoc/mongoc.h>

typedef struct {
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;
} Task112;

void task112_init(Task112 *task) {
    mongoc_init();
    task->client = mongoc_client_new("mongodb://localhost:27017");
    task->database = mongoc_client_get_database(task->client, "testdb");
    task->collection = mongoc_client_get_collection(task->client, "testdb", "users");
}

char* create_document(Task112 *task, const char *name, const char *email, int age) {
    bson_t *document = bson_new();
    bson_oid_t oid;
    bson_error_t error;
    static char id_str[25];
    
    bson_oid_init(&oid, NULL);
    BSON_APPEND_OID(document, "_id", &oid);
    BSON_APPEND_UTF8(document, "name", name);
    BSON_APPEND_UTF8(document, "email", email);
    BSON_APPEND_INT32(document, "age", age);
    
    if (!mongoc_collection_insert_one(task->collection, document, NULL, NULL, &error)) {
        fprintf(stderr, "Error creating document: %s\\n", error.message);
        bson_destroy(document);
        return NULL;
    }
    
    bson_oid_to_string(&oid, id_str);
    bson_destroy(document);
    return id_str;
}

char* read_document(Task112 *task, const char *id) {
    bson_t *query = bson_new();
    bson_oid_t oid;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    static char json_str[1024];
    
    bson_oid_init_from_string(&oid, id);
    BSON_APPEND_OID(query, "_id", &oid);
    
    cursor = mongoc_collection_find_with_opts(task->collection, query, NULL, NULL);
    
    if (mongoc_cursor_next(cursor, &doc)) {
        char *str = bson_as_canonical_extended_json(doc, NULL);
        strncpy(json_str, str, sizeof(json_str) - 1);
        json_str[sizeof(json_str) - 1] = '\\0';
        bson_free(str);
        mongoc_cursor_destroy(cursor);
        bson_destroy(query);
        return json_str;
    }
    
    mongoc_cursor_destroy(cursor);
    bson_destroy(query);
    return NULL;
}

int update_document(Task112 *task, const char *id, const char *name, const char *email, int age) {
    bson_t *query = bson_new();
    bson_t *update = bson_new();
    bson_t *set = bson_new();
    bson_oid_t oid;
    bson_error_t error;
    
    bson_oid_init_from_string(&oid, id);
    BSON_APPEND_OID(query, "_id", &oid);
    
    if (name != NULL) {
        BSON_APPEND_UTF8(set, "name", name);
    }
    if (email != NULL) {
        BSON_APPEND_UTF8(set, "email", email);
    }
    if (age > 0) {
        BSON_APPEND_INT32(set, "age", age);
    }
    
    BSON_APPEND_DOCUMENT(update, "$set", set);
    
    if (!mongoc_collection_update_one(task->collection, query, update, NULL, NULL, &error)) {
        fprintf(stderr, "Error updating document: %s\\n", error.message);
        bson_destroy(query);
        bson_destroy(update);
        bson_destroy(set);
        return 0;
    }
    
    bson_destroy(query);
    bson_destroy(update);
    bson_destroy(set);
    return 1;
}

int delete_document(Task112 *task, const char *id) {
    bson_t *query = bson_new();
    bson_oid_t oid;
    bson_error_t error;
    
    bson_oid_init_from_string(&oid, id);
    BSON_APPEND_OID(query, "_id", &oid);
    
    if (!mongoc_collection_delete_one(task->collection, query, NULL, NULL, &error)) {
        fprintf(stderr, "Error deleting document: %s\\n", error.message);
        bson_destroy(query);
        return 0;
    }
    
    bson_destroy(query);
    return 1;
}

void task112_cleanup(Task112 *task) {
    mongoc_collection_destroy(task->collection);
    mongoc_database_destroy(task->database);
    mongoc_client_destroy(task->client);
    mongoc_cleanup();
}

int main() {
    Task112 task;
    task112_init(&task);
    
    printf("=== Test Case 1: Create Document ===\\n");
    char *id1 = create_document(&task, "John Doe", "john@example.com", 30);
    printf("Created document with ID: %s\\n", id1);
    
    printf("\\n=== Test Case 2: Read Document ===\\n");
    if (id1 != NULL) {
        char *doc = read_document(&task, id1);
        printf("Retrieved document: %s\\n", doc);
    }
    
    printf("\\n=== Test Case 3: Create Multiple Documents ===\\n");
    char *id2 = create_document(&task, "Jane Smith", "jane@example.com", 25);
    char *id3 = create_document(&task, "Bob Johnson", "bob@example.com", 35);
    printf("Created documents with IDs: %s, %s\\n", id2, id3);
    
    printf("\\n=== Test Case 4: Update Document ===\\n");
    if (id1 != NULL) {
        int updated = update_document(&task, id1, "John Updated", NULL, 31);
        printf("Update successful: %d\\n", updated);
        char *updated_doc = read_document(&task, id1);
        printf("Updated document: %s\\n", updated_doc);
    }
    
    printf("\\n=== Test Case 5: Delete Document ===\\n");
    if (id1 != NULL) {
        int deleted = delete_document(&task, id1);
        printf("Delete successful: %d\\n", deleted);
        char *deleted_doc = read_document(&task, id1);
        printf("Document after deletion: %s\\n", deleted_doc ? deleted_doc : "NULL");
    }
    
    task112_cleanup(&task);
    return 0;
}
