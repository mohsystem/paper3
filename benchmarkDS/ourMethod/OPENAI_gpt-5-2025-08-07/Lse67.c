#include <mysql/mysql.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#define MAX_IMAGE_BYTES (10u * 1024u * 1024u) /* 10 MiB */

static int safe_table_name(const char* t) {
    if (!t) return 0;
    size_t n = strlen(t);
    if (n == 0 || n > 64) return 0;
    for (size_t i = 0; i < n; i++) {
        char c = t[i];
        if (!(isalnum((unsigned char)c) || c == '_')) return 0;
    }
    return 1;
}

static int starts_with(const unsigned char* d, size_t len, const unsigned char* pfx, size_t pfxlen) {
    if (len < pfxlen) return 0;
    return memcmp(d, pfx, pfxlen) == 0;
}

static int detect_mime(const unsigned char* data, size_t len, char* out, size_t outlen) {
    if (len >= 8) {
        const unsigned char png[8] = {0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A};
        if (memcmp(data, png, 8) == 0) { snprintf(out, outlen, "image/png"); return 1; }
    }
    if (len >= 3) {
        if (data[0]==0xFF && data[1]==0xD8 && data[2]==0xFF) { snprintf(out, outlen, "image/jpeg"); return 1; }
    }
    if (len >= 6) {
        if (data[0]=='G' && data[1]=='I' && data[2]=='F' && data[3]=='8' && data[5]=='a') {
            snprintf(out, outlen, "image/gif"); return 1;
        }
    }
    if (len >= 2) {
        if (data[0]=='B' && data[1]=='M') { snprintf(out, outlen, "image/bmp"); return 1; }
    }
    if (len >= 12) {
        if (data[0]=='R' && data[1]=='I' && data[2]=='F' && data[3]=='F' &&
            data[8]=='W' && data[9]=='E' && data[10]=='B' && data[11]=='P') {
            snprintf(out, outlen, "image/webp"); return 1;
        }
    }
    return 0;
}

static int ends_with_ci(const char* s, const char* suffix) {
    size_t ls = strlen(s), lf = strlen(suffix);
    if (lf > ls) return 0;
    const char* p = s + (ls - lf);
    for (size_t i = 0; i < lf; i++) {
        if (tolower((unsigned char)p[i]) != tolower((unsigned char)suffix[i])) return 0;
    }
    return 1;
}

static int allowed_extension(const char* rel, const char* mime) {
    if (strcmp(mime, "image/png") == 0) return ends_with_ci(rel, ".png");
    if (strcmp(mime, "image/jpeg") == 0) return ends_with_ci(rel, ".jpg") || ends_with_ci(rel, ".jpeg");
    if (strcmp(mime, "image/gif") == 0) return ends_with_ci(rel, ".gif");
    if (strcmp(mime, "image/bmp") == 0) return ends_with_ci(rel, ".bmp");
    if (strcmp(mime, "image/webp") == 0) return ends_with_ci(rel, ".webp");
    return 0;
}

static int read_image_secure(const char* base_dir, const char* rel_path, unsigned char** out_buf, size_t* out_len, char* out_mime, size_t mime_len) {
    if (!base_dir || !rel_path || !out_buf || !out_len || !out_mime) return 0;
    if (rel_path[0] == '/') {
        fprintf(stderr, "Error: invalid path.\n");
        return 0;
    }
    struct stat st;
    if (lstat(base_dir, &st) != 0) {
        fprintf(stderr, "Error: base directory not accessible.\n");
        return 0;
    }
    if (!S_ISDIR(st.st_mode) || S_ISLNK(st.st_mode)) {
        fprintf(stderr, "Error: base directory invalid.\n");
        return 0;
    }
    int dirfd = open(base_dir, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (dirfd < 0) {
        fprintf(stderr, "Error: cannot open base directory.\n");
        return 0;
    }

    int flags = O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
    int fd = openat(dirfd, rel_path, flags);
    close(dirfd);
    if (fd < 0) {
        fprintf(stderr, "Error: cannot open file.\n");
        return 0;
    }

    struct stat fst;
    if (fstat(fd, &fst) != 0) {
        fprintf(stderr, "Error: cannot stat file.\n");
        close(fd);
        return 0;
    }
    if (!S_ISREG(fst.st_mode)) {
        fprintf(stderr, "Error: not a regular file.\n");
        close(fd);
        return 0;
    }
    if (fst.st_size <= 0 || (size_t)fst.st_size > MAX_IMAGE_BYTES) {
        fprintf(stderr, "Error: invalid file size.\n");
        close(fd);
        return 0;
    }

    size_t cap = (size_t)fst.st_size;
    unsigned char* buf = (unsigned char*)malloc(cap);
    if (!buf) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        close(fd);
        return 0;
    }

    size_t total = 0;
    while (total < cap) {
        ssize_t r = read(fd, buf + total, cap - total);
        if (r < 0) {
            fprintf(stderr, "Error: read failed.\n");
            free(buf);
            close(fd);
            return 0;
        }
        if (r == 0) break;
        total += (size_t)r;
        if (total > MAX_IMAGE_BYTES) {
            fprintf(stderr, "Error: file too large.\n");
            free(buf);
            close(fd);
            return 0;
        }
    }
    close(fd);
    if (total == 0) {
        fprintf(stderr, "Error: empty file.\n");
        free(buf);
        return 0;
    }

    if (!detect_mime(buf, total, out_mime, mime_len)) {
        fprintf(stderr, "Error: unsupported image format.\n");
        free(buf);
        return 0;
    }
    if (!allowed_extension(rel_path, out_mime)) {
        fprintf(stderr, "Error: extension mismatch.\n");
        free(buf);
        return 0;
    }

    *out_buf = buf;
    *out_len = total;
    return 1;
}

int store_image_to_mysql(const char* host, unsigned int port, const char* database,
                         const char* user, const char* password, const char* table_name,
                         const char* base_dir, const char* relative_path, const char* name_in_db,
                         const char* ssl_ca_path /* nullable */) {
    if (!host || !database || !user || !password || !table_name || !base_dir || !relative_path || !name_in_db) {
        fprintf(stderr, "Error: invalid parameters.\n");
        return 0;
    }
    if (!safe_table_name(table_name)) {
        fprintf(stderr, "Error: invalid table name.\n");
        return 0;
    }

    unsigned char* data = NULL;
    size_t data_len = 0;
    char mime[64] = {0};
    if (!read_image_secure(base_dir, relative_path, &data, &data_len, mime, sizeof(mime))) {
        return 0;
    }

    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "Error: mysql init failed.\n");
        free(data);
        return 0;
    }

    unsigned int timeout = 10;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    enum mysql_ssl_mode mode = SSL_MODE_VERIFY_IDENTITY;
    mysql_options(conn, MYSQL_OPT_SSL_MODE, &mode);
    if (ssl_ca_path && *ssl_ca_path) {
        mysql_options(conn, MYSQL_OPT_SSL_CA, ssl_ca_path);
    }

    if (!mysql_real_connect(conn, host, user, password, database, port, NULL, 0)) {
        fprintf(stderr, "Error: database connection failed.\n");
        mysql_close(conn);
        free(data);
        return 0;
    }

    char create_sql[512];
    snprintf(create_sql, sizeof(create_sql),
             "CREATE TABLE IF NOT EXISTS `%s` ("
             "id BIGINT AUTO_INCREMENT PRIMARY KEY,"
             "name VARCHAR(255) NOT NULL,"
             "mime VARCHAR(64) NOT NULL,"
             "data LONGBLOB NOT NULL,"
             "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
             ") CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci", table_name);

    if (mysql_query(conn, create_sql) != 0) {
        fprintf(stderr, "Error: table creation failed.\n");
        mysql_close(conn);
        free(data);
        return 0;
    }

    char insert_sql[256];
    snprintf(insert_sql, sizeof(insert_sql), "INSERT INTO `%s` (name, mime, data) VALUES (?, ?, ?)", table_name);
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "Error: stmt init failed.\n");
        mysql_close(conn);
        free(data);
        return 0;
    }
    if (mysql_stmt_prepare(stmt, insert_sql, (unsigned long)strlen(insert_sql)) != 0) {
        fprintf(stderr, "Error: stmt prepare failed.\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        free(data);
        return 0;
    }

    MYSQL_BIND bind[3];
    memset(bind, 0, sizeof(bind));

    unsigned long name_len = (unsigned long)strlen(name_in_db);
    unsigned long mime_len = (unsigned long)strlen(mime);
    unsigned long blob_len = (unsigned long)data_len;

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)name_in_db;
    bind[0].buffer_length = name_len;
    bind[0].length = &name_len;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)mime;
    bind[1].buffer_length = mime_len;
    bind[1].length = &mime_len;

    bind[2].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[2].buffer = (void*)data;
    bind[2].buffer_length = blob_len;
    bind[2].length = &blob_len;

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        fprintf(stderr, "Error: bind failed.\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        free(data);
        return 0;
    }

    if (mysql_stmt_execute(stmt) != 0) {
        fprintf(stderr, "Error: execute failed.\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        free(data);
        return 0;
    }

    mysql_stmt_close(stmt);
    mysql_close(conn);
    free(data);
    return 1;
}

static void write_file(const char* path, const unsigned char* data, size_t len) {
    // create directories
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    char* p = strrchr(tmp, '/');
    if (p) {
        *p = '\0';
        char acc[1024] = {0};
        size_t L = strlen(tmp);
        for (size_t i = 0; i < L; i++) {
            acc[i] = tmp[i];
            if (acc[i] == '/') {
                mkdir(acc, 0700);
            }
        }
        mkdir(tmp, 0700);
    }
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    if (fd >= 0) {
        size_t off = 0;
        while (off < len) {
            ssize_t w = write(fd, data + off, len - off);
            if (w <= 0) break;
            off += (size_t)w;
        }
        close(fd);
    }
}

static unsigned char* minimal_png(size_t* out_len) {
    static const unsigned char arr[] = {
        0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A,
        0x00,0x00,0x00,0x0D,'I','H','D','R',
        0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,
        0x08,0x06,0x00,0x00,0x00,0x1F,0x15,0xC4,0x89,
        0x00,0x00,0x00,0x0A,'I','D','A','T',
        0x78,0x9C,0x63,0x00,0x01,0x00,0x00,0x05,0x00,0x01,
        0x0D,0x0A,0x2D,0xB4,0x00,0x00,0x00,0x00,'I','E','N','D',
        0xAE,0x42,0x60,0x82
    };
    *out_len = sizeof(arr);
    unsigned char* b = (unsigned char*)malloc(*out_len);
    if (b) memcpy(b, arr, *out_len);
    return b;
}

static unsigned char* minimal_jpeg(size_t* out_len) {
    static const unsigned char arr[] = {
        0xFF,0xD8,0xFF,0xE0,0x00,0x10,'J','F','I','F',0x00,0x01,0x01,0x00,0x00,0x01,0x00,0x01,0x00,0x00,
        0xFF,0xD9
    };
    *out_len = sizeof(arr);
    unsigned char* b = (unsigned char*)malloc(*out_len);
    if (b) memcpy(b, arr, *out_len);
    return b;
}

int main(void) {
    const char* host = getenv("DB_HOST"); if (!host) host = "localhost";
    unsigned int port = getenv("DB_PORT") ? (unsigned int)atoi(getenv("DB_PORT")) : 3306u;
    const char* db = getenv("DB_NAME"); if (!db) db = "test";
    const char* user = getenv("DB_USER"); if (!user) user = "testuser";
    const char* pass = getenv("DB_PASS"); if (!pass) pass = "testpass";
    const char* ssl_ca = getenv("MYSQL_SSL_CA"); // optional
    const char* table = "images_secure";

    char templ[] = "/tmp/safe_images_cXXXXXX";
    char* base = mkdtemp(templ);
    if (!base) {
        fprintf(stderr, "Failed to create temp directory.\n");
        return 1;
    }

    char png[1024], jpg[1024], bad[1024], big[1024];
    snprintf(png, sizeof(png), "%s/%s", base, "pic1.png");
    snprintf(jpg, sizeof(jpg), "%s/%s", base, "nested/dir/pic2.jpg");
    snprintf(bad, sizeof(bad), "%s/%s", base, "note.txt");
    snprintf(big, sizeof(big), "%s/%s", base, "big.webp");

    size_t l; unsigned char* b = minimal_png(&l); if (b) { write_file(png, b, l); free(b); }
    b = minimal_jpeg(&l); if (b) { write_file(jpg, b, l); free(b); }
    const char* hello = "hello";
    write_file(bad, (const unsigned char*)hello, strlen(hello));
    // Create big file
    {
        int fd = open(big, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
        if (fd >= 0) {
            unsigned char* chunk = (unsigned char*)malloc(1024 * 1024);
            if (chunk) {
                memset(chunk, 'A', 1024 * 1024);
                for (int i = 0; i < 10; i++) write(fd, chunk, 1024 * 1024);
                unsigned char z = 0;
                write(fd, &z, 1);
                free(chunk);
            }
            close(fd);
        }
    }

    printf("Test1 (valid PNG): %s\n", store_image_to_mysql(host, port, db, user, pass, table, base, "pic1.png", "png_test", ssl_ca) ? "true" : "false");
    printf("Test2 (valid JPEG): %s\n", store_image_to_mysql(host, port, db, user, pass, table, base, "nested/dir/pic2.jpg", "jpeg_test", ssl_ca) ? "true" : "false");
    printf("Test3 (bad extension): %s\n", store_image_to_mysql(host, port, db, user, pass, table, base, "note.txt", "badext_test", ssl_ca) ? "true" : "false");
    printf("Test4 (path traversal attempt): %s\n", store_image_to_mysql(host, port, db, user, pass, table, base, "../etc/passwd", "traversal", ssl_ca) ? "true" : "false");
    printf("Test5 (too large): %s\n", store_image_to_mysql(host, port, db, user, pass, table, base, "big.webp", "big_test", ssl_ca) ? "true" : "false");

    return 0;
}