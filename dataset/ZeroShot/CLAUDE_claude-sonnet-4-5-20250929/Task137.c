
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typedef struct {
    int id;
    char name[100];
    double price;
    char category[50];
} Product;

typedef struct {
    Product* products;
    int count;
    int capacity;
} ProductList;

ProductList* create_product_list() {
    ProductList* list = (ProductList*)malloc(sizeof(ProductList));
    list->capacity = 10;
    list->count = 0;
    list->products = (Product*)malloc(sizeof(Product) * list->capacity);
    return list;
}

void add_product(ProductList* list, int id, const char* name, double price, const char* category) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->products = (Product*)realloc(list->products, sizeof(Product) * list->capacity);
    }
    
    Product* p = &list->products[list->count];
    p->id = id;
    strncpy(p->name, name, 99);
    p->name[99] = '\\0';
    p->price = price;
    strncpy(p->category, category, 49);
    p->category[49] = '\\0';
    list->count++;
}

void free_product_list(ProductList* list) {
    if (list) {
        free(list->products);
        free(list);
    }
}

void print_product(const Product* p) {
    printf("Product{id=%d, name='%s', price=%.2f, category='%s'}\\n", 
           p->id, p->name, p->price, p->category);
}

sqlite3* initialize_database() {
    sqlite3* db;
    char* errMsg = NULL;
    int rc;
    
    rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return NULL;
    }
    
    const char* create_table = 
        "CREATE TABLE IF NOT EXISTS products ("
        "id INTEGER PRIMARY KEY, "
        "name TEXT NOT NULL, "
        "price REAL NOT NULL, "
        "category TEXT NOT NULL)";
    
    rc = sqlite3_exec(db, create_table, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return NULL;
    }
    
    const char* inserts[] = {
        "INSERT INTO products VALUES (1, 'Laptop', 999.99, 'Electronics')",
        "INSERT INTO products VALUES (2, 'Mouse', 29.99, 'Electronics')",
        "INSERT INTO products VALUES (3, 'Desk Chair', 199.99, 'Furniture')",
        "INSERT INTO products VALUES (4, 'Notebook', 4.99, 'Stationery')",
        "INSERT INTO products VALUES (5, 'Monitor', 299.99, 'Electronics')"
    };
    
    for (int i = 0; i < 5; i++) {
        rc = sqlite3_exec(db, inserts[i], NULL, NULL, &errMsg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\\n", errMsg);
            sqlite3_free(errMsg);
        }
    }
    
    return db;
}

ProductList* query_product_by_id(sqlite3* db, int product_id) {
    ProductList* results = create_product_list();
    const char* query = "SELECT * FROM products WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement\\n");
        return results;
    }
    
    sqlite3_bind_int(stmt, 1, product_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        double price = sqlite3_column_double(stmt, 2);
        const char* category = (const char*)sqlite3_column_text(stmt, 3);
        add_product(results, id, name, price, category);
    }
    
    sqlite3_finalize(stmt);
    return results;
}

ProductList* query_product_by_name(sqlite3* db, const char* product_name) {
    ProductList* results = create_product_list();
    const char* query = "SELECT * FROM products WHERE name LIKE ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement\\n");
        return results;
    }
    
    char search_pattern[200];
    snprintf(search_pattern, 200, "%%%s%%", product_name);
    sqlite3_bind_text(stmt, 1, search_pattern, -1, SQLITE_TRANSIENT);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        double price = sqlite3_column_double(stmt, 2);
        const char* category = (const char*)sqlite3_column_text(stmt, 3);
        add_product(results, id, name, price, category);
    }
    
    sqlite3_finalize(stmt);
    return results;
}

ProductList* query_product_by_category(sqlite3* db, const char* category) {
    ProductList* results = create_product_list();
    const char* query = "SELECT * FROM products WHERE category = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement\\n");
        return results;
    }
    
    sqlite3_bind_text(stmt, 1, category, -1, SQLITE_TRANSIENT);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        double price = sqlite3_column_double(stmt, 2);
        const char* cat = (const char*)sqlite3_column_text(stmt, 3);
        add_product(results, id, name, price, cat);
    }
    
    sqlite3_finalize(stmt);
    return results;
}

int main() {
    sqlite3* db = initialize_database();
    if (!db) {
        fprintf(stderr, "Failed to initialize database\\n");
        return 1;
    }
    
    printf("Test Case 1: Query product by ID = 1\\n");
    ProductList* test1 = query_product_by_id(db, 1);
    for (int i = 0; i < test1->count; i++) {
        print_product(&test1->products[i]);
    }
    free_product_list(test1);
    
    printf("\\nTest Case 2: Query product by name 'Mouse'\\n");
    ProductList* test2 = query_product_by_name(db, "Mouse");
    for (int i = 0; i < test2->count; i++) {
        print_product(&test2->products[i]);
    }
    free_product_list(test2);
    
    printf("\\nTest Case 3: Query products by category 'Electronics'\\n");
    ProductList* test3 = query_product_by_category(db, "Electronics");
    for (int i = 0; i < test3->count; i++) {
        print_product(&test3->products[i]);
    }
    free_product_list(test3);
    
    printf("\\nTest Case 4: Query product by partial name 'ote'\\n");
    ProductList* test4 = query_product_by_name(db, "ote");
    for (int i = 0; i < test4->count; i++) {
        print_product(&test4->products[i]);
    }
    free_product_list(test4);
    
    printf("\\nTest Case 5: Query non-existent product ID = 999\\n");
    ProductList* test5 = query_product_by_id(db, 999);
    if (test5->count == 0) {
        printf("No products found\\n");
    } else {
        for (int i = 0; i < test5->count; i++) {
            print_product(&test5->products[i]);
        }
    }
    free_product_list(test5);
    
    sqlite3_close(db);
    return 0;
}
