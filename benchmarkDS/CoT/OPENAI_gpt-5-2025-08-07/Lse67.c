/* C implementation using MySQL C API (libmysqlclient)
   Chain-of-Through process (brief):
   1) Goal: store image file as BLOB into MySQL.
   2) Security: validate identifier, use prepared statements, enforce file size limit.
   3) Secure coding: robust error handling, transactions, resource cleanup.
   4) Review: prevent SQL injection, check lengths and return codes.
   5) Output: final function plus 5 test cases.
*/

#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/stat.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#define MAX_FILE_SIZE_BYTES_C (10 * 1024 * 1024) /* 10MB */

static bool safe_identifier_c(const char* s) {
    if (!s) return false;
    size_t n = strlen(s);
    if (n == 0 || n > 64) return false;
    for (size_t i = 0; i < n; ++i) {
        char c = s[i];
        if (!( (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') ||
               (c == '_') )) return false;
    }
    return true;
}

static char* basename_c(const char* path) {
    const char* p = path;
    const char* last = p;
    while (*p) {
        if (*p == '/' || *p == '\\') last = p + 1;
        p++;
    }
    size_t len = strlen(last);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, last, len);
    out[len] = '\0';
    return out;
}

static char* mime_from_ext_c(const char* path) {
    size_t len = strlen(path);
    const char* ext = NULL;
    for (ssize_t i = (ssize_t)len - 1; i >= 0; --i) {
        if (path[i] == '.') { ext = path + i; break; }
        if (path[i] == '/' || path[i] == '\\') break;
    }
    const char* mime = "application/octet-stream";
    if (ext) {
        if (strcasecmp(ext, ".png") == 0) mime = "image/png";
        else if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0) mime = "image/jpeg";
        else if (strcasecmp(ext, ".gif") == 0) mime = "image/gif";
        else if (strcasecmp(ext, ".bmp") == 0) mime = "image/bmp";
        else if (strcasecmp(ext, ".webp") == 0) mime = "image/webp";
    }
    char* out = (char*)malloc(strlen(mime) + 1);
    if (!out) return NULL;
    strcpy(out, mime);
    return out;
}

static bool read_file_c(const char* path, unsigned char** buf, unsigned long* len) {
    struct stat st;
    if (stat(path, &st) != 0) return false;
    if (!S_ISREG(st.st_mode)) return false;
    if (st.st_size <= 0 || st.st_size > MAX_FILE_SIZE_BYTES_C) return false;

    FILE* f = fopen(path, "rb");
    if (!f) return false;
    unsigned char* data = (unsigned char*)malloc((size_t)st.st_size);
    if (!data) { fclose(f); return false; }
    size_t rd = fread(data, 1, (size_t)st.st_size, f);
    fclose(f);
    if (rd != (size_t)st.st_size) {
        free(data);
        return false;
    }
    *buf = data;
    *len = (unsigned long)rd;
    return true;
}

bool store_image_to_mysql_c(
    const char* host,
    unsigned int port,
    const char* user,
    const char* pass,
    const char* db,
    const char* table,
    const char* record_id,
    const char* file_path
) {
    if (!safe_identifier_c(table)) return false;
    if (!record_id || !*record_id) return false;

    unsigned char* data = NULL;
    unsigned long data_len = 0;
    if (!read_file_c(file_path, &data, &data_len)) return false;

    char* filename = basename_c(file_path);
    char* mime = mime_from_ext_c(file_path);
    if (!filename || !mime) {
        free(data);
        if (filename) free(filename);
        if (mime) free(mime);
        return false;
    }

    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        free(data); free(filename); free(mime);
        return false;
    }

    unsigned int timeout_sec = 8;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout_sec);
    mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &timeout_sec);
    mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &timeout_sec);

    if (!mysql_real_connect(conn, host, user, pass, db, port, NULL, 0)) {
        mysql_close(conn);
        free(data); free(filename); free(mime);
        return false;
    }

    if (mysql_autocommit(conn, 0) != 0) {
        mysql_close(conn);
        free(data); free(filename); free(mime);
        return false;
    }

    bool ok = false;

    // Create table
    char create_sql[512];
    // Safe because table validated as [A-Za-z0-9_]+ and <= 64
    snprintf(create_sql, sizeof(create_sql),
             "CREATE TABLE IF NOT EXISTS `%s` ("
             "id VARCHAR(128) PRIMARY KEY,"
             "filename VARCHAR(255) NOT NULL,"
             "mime_type VARCHAR(100) NOT NULL,"
             "size_bytes BIGINT NOT NULL,"
             "data LONGBLOB NOT NULL)",
             table);

    if (mysql_query(conn, create_sql) != 0) {
        mysql_rollback(conn);
        mysql_autocommit(conn, 1);
        mysql_close(conn);
        free(data); free(filename); free(mime);
        return false;
    }

    char insert_sql[256];
    snprintf(insert_sql, sizeof(insert_sql),
             "REPLACE INTO `%s` (id, filename, mime_type, size_bytes, data) VALUES (?,?,?,?,?)",
             table);

    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        mysql_rollback(conn);
        mysql_autocommit(conn, 1);
        mysql_close(conn);
        free(data); free(filename); free(mime);
        return false;
    }
    if (mysql_stmt_prepare(stmt, insert_sql, (unsigned long)strlen(insert_sql)) != 0) {
        mysql_stmt_close(stmt);
        mysql_rollback(conn);
        mysql_autocommit(conn, 1);
        mysql_close(conn);
        free(data); free(filename); free(mime);
        return false;
    }

    MYSQL_BIND bind[5];
    memset(bind, 0, sizeof(bind));

    unsigned long id_len = (unsigned long)strlen(record_id);
    unsigned long fn_len = (unsigned long)strlen(filename);
    unsigned long mime_len = (unsigned long)strlen(mime);
    long long size_bytes = (long long)data_len;

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)record_id;
    bind[0].length = &id_len;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)filename;
    bind[1].length = &fn_len;

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void*)mime;
    bind[2].length = &mime_len;

    bind[3].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[3].buffer = (void*)&size_bytes;

    bind[4].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[4].buffer = (void*)data;
    bind[4].length = &data_len;

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        mysql_rollback(conn);
        mysql_autocommit(conn, 1);
        mysql_close(conn);
        free(data); free(filename); free(mime);
        return false;
    }

    if (mysql_stmt_execute(stmt) == 0) {
        ok = true;
        mysql_commit(conn);
    } else {
        mysql_rollback(conn);
    }

    mysql_stmt_close(stmt);
    mysql_autocommit(conn, 1);
    mysql_close(conn);
    free(data); free(filename); free(mime);
    return ok;
}

static int write_tiny_png(const char* path) {
    unsigned char bytes[] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0x00,0x00,0x00,0x00,0x49,0x45,0x4E,0x44,0xAE,0x42,0x60,0x82};
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    size_t w = fwrite(bytes, 1, sizeof(bytes), f);
    fclose(f);
    return (w == sizeof(bytes)) ? 0 : -1;
}

static char* make_temp_name(const char* suffix) {
    char* name = (char*)malloc(64);
    if (!name) return NULL;
#ifdef _WIN32
    snprintf(name, 64, "imgc_%u%s", (unsigned)rand(), suffix);
#else
    snprintf(name, 64, "imgc_%u%s", (unsigned)getpid(), suffix);
#endif
    return name;
}

int main(void) {
    const char* host = getenv("DB_HOST") ? getenv("DB_HOST") : "127.0.0.1";
    unsigned int port = getenv("DB_PORT") ? (unsigned int)atoi(getenv("DB_PORT")) : 3306;
    const char* user = getenv("DB_USER") ? getenv("DB_USER") : "root";
    const char* pass = getenv("DB_PASS") ? getenv("DB_PASS") : "password";
    const char* db = getenv("DB_NAME") ? getenv("DB_NAME") : "test";
    const char* table = getenv("DB_TABLE") ? getenv("DB_TABLE") : "images_secure_c";

    char* p1 = make_temp_name(".png");
    char* p2 = make_temp_name(".jpg");
    char* p3 = make_temp_name(".gif");
    char* p4 = make_temp_name(".webp");
    char* p5 = make_temp_name(".bmp");

    if (p1 && p2 && p3 && p4 && p5) {
        write_tiny_png(p1);
        write_tiny_png(p2);
        write_tiny_png(p3);
        write_tiny_png(p4);
        write_tiny_png(p5);

        store_image_to_mysql_c(host, port, user, pass, db, table, "c_img1", p1);
        store_image_to_mysql_c(host, port, user, pass, db, table, "c_img2", p2);
        store_image_to_mysql_c(host, port, user, pass, db, table, "c_img3", p3);
        store_image_to_mysql_c(host, port, user, pass, db, table, "c_img4", p4);
        store_image_to_mysql_c(host, port, user, pass, db, table, "c_img5", p5);

        remove(p1); remove(p2); remove(p3); remove(p4); remove(p5);
    }

    if (p1) free(p1);
    if (p2) free(p2);
    if (p3) free(p3);
    if (p4) free(p4);
    if (p5) free(p5);

    return 0;
}