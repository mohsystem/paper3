#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

#define UPLOAD_DIR "uploads"

static const char* DB_HOST = "127.0.0.1";
static unsigned int DB_PORT = 3306;
static const char* DB_USER = "root";
static const char* DB_PASS = "";
static const char* DB_NAME = "test";

typedef struct {
    unsigned char* data;
    size_t len;
} Buffer;

static void ensure_uploads_dir() {
#ifdef _WIN32
    _mkdir(UPLOAD_DIR);
#else
    mkdir(UPLOAD_DIR, 0755);
#endif
}

static MYSQL* get_connection() {
    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "mysql_init failed\n");
        exit(1);
    }
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0)) {
        fprintf(stderr, "mysql_real_connect failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }
    return conn;
}

static void ensure_table(MYSQL* conn) {
    const char* ddl =
        "CREATE TABLE IF NOT EXISTS pdf_uploads ("
        " id BIGINT AUTO_INCREMENT PRIMARY KEY,"
        " filename VARCHAR(255) NOT NULL,"
        " filepath VARCHAR(1024) NOT NULL,"
        " uploaded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";
    if (mysql_query(conn, ddl) != 0) {
        fprintf(stderr, "DDL error: %s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }
}

static char* sanitize_filename(const char* name) {
    size_t n = strlen(name);
    char* out = (char*)malloc(n + 5);
    size_t j = 0;
    for (size_t i = 0; i < n; ++i) {
        char c = name[i];
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '_' || c == '-') {
            out[j++] = c;
        } else {
            out[j++] = '_';
        }
    }
    out[j] = '\0';
    const char* dot = strrchr(out, '.');
    if (!dot || strcasecmp(dot, ".pdf") != 0) {
        strcpy(out + j, ".pdf");
    }
    return out;
}

static char* build_unique_name(const char* base) {
    char* out = (char*)malloc(64 + strlen(base));
#ifdef _WIN32
    unsigned int r = (unsigned int)rand();
#else
    unsigned int r = (unsigned int)random();
#endif
    snprintf(out, 64 + strlen(base), "%lu-%u-%s", (unsigned long)time(NULL), r, base);
    return out;
}

static long upload_pdf(const unsigned char* data, size_t len, const char* original_filename) {
    ensure_uploads_dir();
    char* safe = sanitize_filename(original_filename ? original_filename : "upload.pdf");
    char* unique = build_unique_name(safe);

    char path[2048];
#ifdef _WIN32
    _snprintf(path, sizeof(path), "%s\\%s", UPLOAD_DIR, unique);
#else
    snprintf(path, sizeof(path), "%s/%s", UPLOAD_DIR, unique);
#endif

    FILE* f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "Failed to open file for write: %s\n", path);
        free(safe); free(unique);
        exit(1);
    }
    fwrite(data, 1, len, f);
    fclose(f);

    MYSQL* conn = get_connection();
    ensure_table(conn);

    // escape strings
    char* esc_name = (char*)malloc(strlen(original_filename) * 2 + 1);
    unsigned long n1 = mysql_real_escape_string(conn, esc_name, original_filename, (unsigned long)strlen(original_filename));
    esc_name[n1] = '\0';

    char* esc_path = (char*)malloc(strlen(path) * 2 + 1);
    unsigned long n2 = mysql_real_escape_string(conn, esc_path, path, (unsigned long)strlen(path));
    esc_path[n2] = '\0';

    char* sql = (char*)malloc(n1 + n2 + 128);
    sprintf(sql, "INSERT INTO pdf_uploads (filename, filepath) VALUES ('%s', '%s')", esc_name, esc_path);

    if (mysql_query(conn, sql) != 0) {
        fprintf(stderr, "Insert error: %s\n", mysql_error(conn));
        mysql_close(conn);
        free(safe); free(unique); free(esc_name); free(esc_path); free(sql);
        exit(1);
    }
    long id = (long)mysql_insert_id(conn);

    mysql_close(conn);
    free(safe); free(unique); free(esc_name); free(esc_path); free(sql);
    return id;
}

static Buffer download_pdf(long id) {
    MYSQL* conn = get_connection();
    char sql[128];
    snprintf(sql, sizeof(sql), "SELECT filepath FROM pdf_uploads WHERE id=%ld", id);
    if (mysql_query(conn, sql) != 0) {
        fprintf(stderr, "Select error: %s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        fprintf(stderr, "Store result error: %s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row || !row[0]) {
        fprintf(stderr, "Record not found\n");
        mysql_free_result(res);
        mysql_close(conn);
        exit(1);
    }
    const char* path = row[0];
    mysql_free_result(res);
    mysql_close(conn);

    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "File not found: %s\n", path);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* buf = (unsigned char*)malloc(sz);
    fread(buf, 1, sz, f);
    fclose(f);

    Buffer b = { buf, (size_t)sz };
    return b;
}

static Buffer make_minimal_pdf(const char* message) {
    // simple minimal PDF with message in a comment
    const char* header = "%PDF-1.4\n";
    const char* obj1 = "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";
    const char* obj2 = "2 0 obj\n<< /Type /Pages /Count 0 >>\nendobj\n";
    const char* trailer = "trailer\n<< /Root 1 0 R >>\n%%EOF\n";

    size_t total = strlen(header) + 2 + strlen(message) + strlen(obj1) + strlen(obj2) + strlen(trailer) + 4;
    unsigned char* buf = (unsigned char*)malloc(total);
    size_t off = 0;
    memcpy(buf + off, header, strlen(header)); off += strlen(header);
    buf[off++] = '%'; buf[off++] = ' ';
    memcpy(buf + off, message, strlen(message)); off += strlen(message);
    buf[off++] = '\n';
    memcpy(buf + off, obj1, strlen(obj1)); off += strlen(obj1);
    memcpy(buf + off, obj2, strlen(obj2)); off += strlen(obj2);
    memcpy(buf + off, trailer, strlen(trailer)); off += strlen(trailer);
    Buffer b = { buf, off };
    return b;
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Test PDF %d", i);
        Buffer pdf = make_minimal_pdf(msg);
        char fname[32];
        snprintf(fname, sizeof(fname), "file%d.pdf", i);
        long id = upload_pdf(pdf.data, pdf.len, fname);
        Buffer out = download_pdf(id);
        printf("Inserted id=%ld, downloaded bytes=%zu\n", id, out.len);
        free(pdf.data);
        free(out.data);
    }
    return 0;
}