
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mysql/mysql.h>

#define MAX_USERNAME_LEN 50
#define MAX_EMAIL_LEN 100
#define MAX_FULLNAME_LEN 100

typedef struct {
    int id;
    char username[MAX_USERNAME_LEN + 1];
    char email[MAX_EMAIL_LEN + 1];
    char full_name[MAX_FULLNAME_LEN + 1];
} Customer;

int validate_username(const char* username) {
    if (username == NULL || strlen(username) == 0 || strlen(username) > MAX_USERNAME_LEN) {
        fprintf(stderr, "Error: Username is empty or too long\\n");
        return 0;
    }
    
    // Check if username contains only alphanumeric and underscore
    for (size_t i = 0; i < strlen(username); i++) {
        if (!isalnum(username[i]) && username[i] != '_') {
            fprintf(stderr, "Error: Invalid username format\\n");
            return 0;
        }
    }
    
    return 1;
}

Customer* get_customer_by_username(const char* customer_username) {
    // Input validation
    if (!validate_username(customer_username)) {
        return NULL;
    }
    
    MYSQL* conn;
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[1];
    MYSQL_BIND result[4];
    Customer* customer = NULL;
    
    // Initialize MySQL
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "MySQL initialization failed\\n");
        return NULL;
    }
    
    // Connect to database
    if (mysql_real_connect(conn, "localhost", "root", "password", "customerdb", 0, NULL, 0) == NULL) {
        fprintf(stderr, "Database connection failed: %s\\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }
    
    // Prepare statement
    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "Statement initialization failed\\n");
        mysql_close(conn);
        return NULL;
    }
    
    const char* query = "SELECT id, username, email, full_name FROM customer WHERE username = ?";
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Statement preparation failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return NULL;
    }
    
    // Bind input parameter
    memset(bind, 0, sizeof(bind));
    unsigned long username_length = strlen(customer_username);
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)customer_username;
    bind[0].buffer_length = username_length;
    bind[0].length = &username_length;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "Parameter binding failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return NULL;
    }
    
    // Execute query
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Query execution failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return NULL;
    }
    
    // Allocate memory for customer
    customer = (Customer*)malloc(sizeof(Customer));
    if (!customer) {
        fprintf(stderr, "Memory allocation failed\\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return NULL;
    }
    
    // Bind results
    memset(result, 0, sizeof(result));
    unsigned long length[4];
    my_bool is_null[4];
    
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &customer->id;
    result[0].is_null = &is_null[0];
    result[0].length = &length[0];
    
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = customer->username;
    result[1].buffer_length = MAX_USERNAME_LEN;
    result[1].is_null = &is_null[1];
    result[1].length = &length[1];
    
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = customer->email;
    result[2].buffer_length = MAX_EMAIL_LEN;
    result[2].is_null = &is_null[2];
    result[2].length = &length[2];
    
    result[3].buffer_type = MYSQL_TYPE_STRING;
    result[3].buffer = customer->full_name;
    result[3].buffer_length = MAX_FULLNAME_LEN;
    result[3].is_null = &is_null[3];
    result[3].length = &length[3];
    
    if (mysql_stmt_bind_result(stmt, result)) {
        fprintf(stderr, "Result binding failed: %s\\n", mysql_stmt_error(stmt));
        free(customer);
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return NULL;
    }
    
    // Fetch result
    int fetch_result = mysql_stmt_fetch(stmt);
    if (fetch_result == 0) {
        customer->username[length[1]] = '\\0';
        customer->email[length[2]] = '\\0';
        customer->full_name[length[3]] = '\\0';
    } else {
        printf("No customer found with username: %s\\n", customer_username);
        free(customer);
        customer = NULL;
    }
    
    // Cleanup
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return customer;
}

void print_customer(const Customer* customer) {
    if (customer) {
        printf("Customer{id=%d, username='%s', email='%s', full_name='%s'}\\n",
               customer->id, customer->username, customer->email, customer->full_name);
    }
}

int main(int argc, char* argv[]) {
    const char* test_usernames[] = {"john_doe", "jane_smith", "admin123", "test_user", "nonexistent"};
    int num_tests = 5;
    
    if (argc > 1) {
        // Use command line argument
        Customer* customer = get_customer_by_username(argv[1]);
        if (customer) {
            printf("Retrieved: ");
            print_customer(customer);
            free(customer);
        }
    } else {
        // Run test cases
        printf("Running test cases...\\n\\n");
        for (int i = 0; i < num_tests; i++) {
            printf("Testing username: %s\\n", test_usernames[i]);
            Customer* customer = get_customer_by_username(test_usernames[i]);
            if (customer) {
                printf("Result: ");
                print_customer(customer);
                free(customer);
            }
            printf("---\\n");
        }
    }
    
    return 0;
}
