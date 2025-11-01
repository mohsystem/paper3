#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

static bool valid_identifier(const char* s) {
    if (s == NULL || *s == '\0') return false;
    for (const char* p = s; *p; ++p) {
        if (!(isalnum((unsigned char)*p) || *p == '_')) return false;
    }
    return true;
}

static bool exec_query(MYSQL* conn, const char* q) {
    if (mysql_query(conn, q) != 0) {
        fprintf(stderr, "MySQL query error: %s\n", mysql_error(conn));
        return false;
    }
    return true;
}

static bool read_file(const char* path, unsigned char** out_buf, unsigned long* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "File not found: %s\n", path);
        return false;
    }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return false; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return false; }
    rewind(f);
    unsigned char* buf = (unsigned char*)malloc((size_t)sz);
    if (!buf) { fclose(f); return false; }
    size_t n = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if ((long)n != sz) { free(buf); return false; }
    *out_buf = buf;
    *out_len = (unsigned long)sz;
    return true;
}

bool store_image(const char* file_path,
                 const char* host,
                 const char* user,
                 const char* pass,
                 const char* db,
                 unsigned int port,
                 const char* table,
                 int id)
{
    if (!valid_identifier(table)) {
        fprintf(stderr, "Invalid table name\n");
        return false;
    }

    unsigned char* data = NULL;
    unsigned long data_len = 0;
    if (!read_file(file_path, &data, &data_len)) {
        return false;
    }

    const char* filename = strrchr(file_path, '/');
#ifdef _WIN32
    const char* filename2 = strrchr(file_path, '\\');
    if (filename2 && (!filename || filename2 > filename)) filename = filename2;
#endif
    filename = (filename) ? filename + 1 : file_path;
    unsigned long filename_len = (unsigned long)strlen(filename);

    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "mysql_init failed\n");
        free(data);
        return false;
    }
    if (!mysql_real_connect(conn, host, user, pass, db, port, NULL, 0)) {
        fprintf(stderr, "MySQL connect error: %s\n", mysql_error(conn));
        mysql_close(conn);
        free(data);
        return false;
    }

    char create_sql[512];
    snprintf(create_sql, sizeof(create_sql),
             "CREATE TABLE IF NOT EXISTS %s (id INT PRIMARY KEY, filename VARCHAR(255), data LONGBLOB)", table);
    if (!exec_query(conn, create_sql)) {
        mysql_close(conn);
        free(data);
        return false;
    }

    char insert_sql[512];
    snprintf(insert_sql, sizeof(insert_sql),
             "INSERT INTO %s (id, filename, data) VALUES (?, ?, ?) "
             "ON DUPLICATE KEY UPDATE filename=VALUES(filename), data=VALUES(data)", table);

    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init failed\n");
        mysql_close(conn);
        free(data);
        return false;
    }
    if (mysql_stmt_prepare(stmt, insert_sql, (unsigned long)strlen(insert_sql)) != 0) {
        fprintf(stderr, "mysql_stmt_prepare error: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        free(data);
        return false;
    }

    MYSQL_BIND bind[3];
    memset(bind, 0, sizeof(bind));

    // id
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char*)&id;

    // filename
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char*)filename;
    bind[1].buffer_length = filename_len;
    bind[1].length = &filename_len;

    // data
    bind[2].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[2].buffer = (char*)data;
    bind[2].buffer_length = data_len;
    bind[2].length = &data_len;

    bool ok = true;
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        fprintf(stderr, "mysql_stmt_bind_param error: %s\n", mysql_stmt_error(stmt));
        ok = false;
    } else if (mysql_stmt_execute(stmt) != 0) {
        fprintf(stderr, "mysql_stmt_execute error: %s\n", mysql_stmt_error(stmt));
        ok = false;
    }

    mysql_stmt_close(stmt);
    mysql_close(conn);
    free(data);
    return ok;
}

int main(void) {
    const char* host = "localhost";
    const char* user = "root";
    const char* pass = "root";
    const char* db   = "testdb";
    unsigned int port = 3306;
    const char* table = "images";

    printf("Test 1: %s\n", store_image("test1.jpg", host, user, pass, db, port, table, 1) ? "ok" : "fail");
    printf("Test 2: %s\n", store_image("test2.png", host, user, pass, db, port, table, 2) ? "ok" : "fail");
    printf("Test 3: %s\n", store_image("test3.gif", host, user, pass, db, port, table, 3) ? "ok" : "fail");
    printf("Test 4: %s\n", store_image("test4.bmp", host, user, pass, db, port, table, 4) ? "ok" : "fail");
    printf("Test 5: %s\n", store_image("test5.webp", host, user, pass, db, port, table, 5) ? "ok" : "fail");
    return 0;
}