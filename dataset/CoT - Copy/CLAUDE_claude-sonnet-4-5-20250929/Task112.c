
/*
 * Note: MongoDB does not have an official C driver that is as straightforward
 * as the drivers for Java, Python, and C++. The official MongoDB C driver
 * (libmongoc) requires external library installation and complex setup.
 * 
 * Below is a conceptual implementation showing how you would structure
 * the code using the libmongoc library. To compile and run this code,
 * you would need to:
 * 1. Install libmongoc and libbson
 * 2. Link against these libraries during compilation
 * 
 * Compilation example:
 * gcc -o task112 task112.c $(pkg-config --cflags --libs libmongoc-1.0)
 */

#include <mongoc/mongoc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <time.h>

typedef struct {
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;
} Task112;

int validate_email(const char *email) {
    regex_t regex;
    int ret;
    
    ret = regcomp(&regex, "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+$", REG_EXTENDED);
    if (ret) {
        return 0;
    }
    
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return !ret;
}

char* trim(const char *str) {
    if (str == NULL) return NULL;
    
    while (*str == ' ') str++;
    
    if (*str == 0) return strdup("");
    
    const char *end = str + strlen(str) - 1;
    while (end > str && *end == ' ') end--;
    
    size_t len = end - str + 1;
    char *trimmed = (char*)malloc(len + 1);
    if (trimmed) {
        memcpy(trimmed, str, len);
        trimmed[len] = '\\0';
    }
    return trimmed;
}

Task112* task112_init(const char *connection_string, const char *db_name, 
                      const char *collection_name) {
    Task112 *task = (Task112*)malloc(sizeof(Task112));
    if (!task) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    mongoc_init();
    
    bson_error_t error;
    task->client = mongoc_client_new(connection_string);
    if (!task->client) {
        fprintf(stderr, "Failed to create MongoDB client\\n");
        free(task);
        return NULL;
    }
    
    task->database = mongoc_client_get_database(task->client, db_name);
    task->collection = mongoc_client_get_collection(task->client, db_name, collection_name);
    
    return task;
}

char* create_document(Task112 *task, const char *name, int age, const char *email) {
    if (!task || !name || !email) {
        fprintf(stderr, "Invalid parameters\\n");
        return NULL;
    }
    
    char *trimmed_name = trim(name);
    char *trimmed_email = trim(email);
    
    // Input validation
    if (strlen(trimmed_name) == 0) {
        fprintf(stderr, "Name cannot be empty\\n");
        free(trimmed_name);
        free(trimmed_email);
        return NULL;
    }
    
    if (age < 0 || age > 150) {
        fprintf(stderr, "Age must be between 0 and 150\\n");
        free(trimmed_name);
        free(trimmed_email);
        return NULL;
    }
    
    if (!validate_email(trimmed_email)) {
        fprintf(stderr, "Invalid email format\\n");
        free(trimmed_name);
        free(trimmed_email);
        return NULL;
    }
    
    bson_t *doc = bson_new();
    bson_oid_t oid;
    bson_oid_init(&oid, NULL);
    
    BSON_APPEND_OID(doc, "_id", &oid);
    BSON_APPEND_UTF8(doc, "name", trimmed_name);
    BSON_APPEND_INT32(doc, "age", age);
    BSON_APPEND_UTF8(doc, "email", trimmed_email);
    BSON_APPEND_INT64(doc, "createdAt", (int64_t)time(NULL));
    
    bson_error_t error;
    if (!mongoc_collection_insert_one(task->collection, doc, NULL, NULL, &error)) {
        fprintf(stderr, "Insert failed: %s\\n", error.message);
        bson_destroy(doc);
        free(trimmed_name);
        free(trimmed_email);
        return NULL;
    }
    
    char *id_str = (char*)malloc(25);
    bson_oid_to_string(&oid, id_str);
    
    bson_destroy(doc);
    free(trimmed_name);
    free(trimmed_email);
    
    return id_str;
}

char* read_document(Task112 *task, const char *id) {
    if (!task || !id || strlen(id) != 24) {
        fprintf(stderr, "Invalid ObjectId format\\n");
        return NULL;
    }
    
    bson_oid_t oid;
    bson_oid_init_from_string(&oid, id);
    
    bson_t *query = bson_new();
    BSON_APPEND_OID(query, "_id", &oid);
    
    const bson_t *doc;
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(
        task->collection, query, NULL, NULL);
    
    char *result = NULL;
    if (mongoc_cursor_next(cursor, &doc)) {
        result = bson_as_canonical_extended_json(doc, NULL);
    }
    
    mongoc_cursor_destroy(cursor);
    bson_destroy(query);
    
    return result;
}

void read_all_documents(Task112 *task) {
    if (!task) return;
    
    bson_t *query = bson_new();
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(
        task->collection, query, NULL, NULL);
    
    const bson_t *doc;
    while (mongoc_cursor_next(cursor, &doc)) {
        char *str = bson_as_canonical_extended_json(doc, NULL);
        printf("%s\\n", str);
        bson_free(str);
    }
    
    mongoc_cursor_destroy(cursor);
    bson_destroy(query);
}

int update_document(Task112 *task, const char *id, const char *name, 
                   int age, const char *email) {
    if (!task || !id || strlen(id) != 24) {
        fprintf(stderr, "Invalid ObjectId format\\n");
        return 0;
    }
    
    // Input validation
    if (name && strlen(trim(name)) == 0) {
        fprintf(stderr, "Name cannot be empty\\n");
        return 0;
    }
    
    if (age < 0 || age > 150) {
        fprintf(stderr, "Age must be between 0 and 150\\n");
        return 0;
    }
    
    if (email && !validate_email(email)) {
        fprintf(stderr, "Invalid email format\\n");
        return 0;
    }
    
    bson_oid_t oid;
    bson_oid_init_from_string(&oid, id);
    
    bson_t *query = bson_new();
    BSON_APPEND_OID(query, "_id", &oid);
    
    bson_t *update = bson_new();
    bson_t child;
    BSON_APPEND_DOCUMENT_BEGIN(update, "$set", &child);
    
    if (name) BSON_APPEND_UTF8(&child, "name", name);
    if (age > 0) BSON_APPEND_INT32(&child, "age", age);
    if (email) BSON_APPEND_UTF8(&child, "email", email);
    BSON_APPEND_INT64(&child, "updatedAt", (int64_t)time(NULL));
    
    bson_append_document_end(update, &child);
    
    bson_error_t error;
    int result = mongoc_collection_update_one(
        task->collection, query, update, NULL, NULL, &error);
    
    bson_destroy(query);
    bson_destroy(update);
    
    return result;
}

int delete_document(Task112 *task, const char *id) {
    if (!task || !id || strlen(id) != 24) {
        fprintf(stderr, "Invalid ObjectId format\\n");
        return 0;
    }
    
    bson_oid_t oid;
    bson_oid_init_from_string(&oid, id);
    
    bson_t *query = bson_new();
    BSON_APPEND_OID(query, "_id", &oid);
    
    bson_error_t error;
    int result = mongoc_collection_delete_one(
        task->collection, query, NULL, NULL, &error);
    
    bson_destroy(query);
    
    return result;
}

void task112_cleanup(Task112 *task) {
    if (task) {
        if (task->collection) mongoc_collection_destroy(task->collection);
        if (task->database) mongoc_database_destroy(task->database);
        if (task->client) mongoc_client_destroy(task->client);
        free(task);
    }
    mongoc_cleanup();
}

int main(void) {
    const char *connection_string = "mongodb://localhost:27017";
    const char *db_name = "testdb";
    const char *collection_name = "users";
    
    Task112 *task = task112_init(connection_string, db_name, collection_name);
    if (!task) {
        fprintf(stderr, "Failed to initialize Task112\\n");
        return 1;
    }
    
    printf("=== Test Case 1: Create Documents ===\\n");
    char *id1 = create_document(task, "John Doe", 30, "john@example.com");
    if (id1) {
        printf("Created document with ID: %s\\n", id1);
    }
    
    char *id2 = create_document(task, "Jane Smith", 25, "jane@example.com");
    if (id2) {
        printf("Created document with ID: %s\\n", id2);
    }
    
    printf("\\n=== Test Case 2: Read Document ===\\n");
    if (id1) {
        char *doc = read_document(task, id1);
        if (doc) {
            printf("Read document: %s\\n", doc);
            bson_free(doc);
        }
    }
    
    printf("\\n=== Test Case 3: Read All Documents ===\\n");
    read_all_documents(task);
    
    printf("\\n=== Test Case 4: Update Document ===\\n");
    if (id1) {
        int updated = update_document(task, id1, "John Updated", 31, "johnupdated@example.com");
        printf("Update successful: %s\\n", updated ? "true" : "false");
    }
    
    printf("\\n=== Test Case 5: Delete Document ===\\n");
    if (id2) {
        int deleted = delete_document(task, id2);
        printf("Delete successful: %s\\n", deleted ? "true" : "false");
    }
    
    printf("\\n=== Final State ===\\n");
    read_all_documents(task);
    
    if (id1) free(id1);
    if (id2) free(id2);
    
    task112_cleanup(task);
    
    return 0;
}
