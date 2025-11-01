#include <mysql/mysql.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_FILE_SIZE (20 * 1024 * 1024)
#define TABLE_NAME "images"

static int file_exists_regular(const char* path, size_t* out_size) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    if (!S_ISREG(st.st_mode)) return 0;
    *out_size = (size_t)st.st_size;
    return 1;
}

static char* detect_image_mime(const unsigned char* head, size_t head_len, const char* filename) {
    if (head_len >= 8 &&
        head[0]==0x89 && head[1]==0x50 && head[2]==0x4E && head[3]==0x47 &&
        head[4]==0x0D && head[5]==0x0A && head[6]==0x1A && head[7]==0x0A) {
        return strdup("image/png");
    }
    if (head_len >= 3 && head[0]==0xFF && head[1]==0xD8 && head[2]==0xFF) {
        return strdup("image/jpeg");
    }
    if (head_len >= 6 &&
        head[0]=='G' && head[1]=='I' && head[2]=='F' &&
        head[3]=='8' && (head[4]=='7' || head[4]=='9') && head[5]=='a') {
        return strdup("image/gif");
    }
    if (head_len >= 12 &&
        head[0]=='R' && head[1]=='I' && head[2]=='F' && head[3]=='F' &&
        head[8]=='W' && head[9]=='E' && head[10]=='B' && head[11]=='P') {
        return strdup("image/webp");
    }
    if (head_len >= 2 && head[0]=='B' && head[1]=='M') {
        return strdup("image/bmp");
    }
    if (head_len >= 4 &&
        ((head[0]=='I' && head[1]=='I' && head[2]==0x2A && head[3]==0x00) ||
         (head[0]=='M' && head[1]=='M' && head[2]==0x00 && head[3]==0x2A))) {
        return strdup("image/tiff");
    }
    // fallback by extension
    const char* dot = strrchr(filename, '.');
    if (dot) {
        if (!strcasecmp(dot, ".jpg") || !strcasecmp(dot, ".jpeg")) return strdup("image/jpeg");
        if (!strcasecmp(dot, ".png")) return strdup("image/png");
        if (!strcasecmp(dot, ".gif")) return strdup("image/gif");
        if (!strcasecmp(dot, ".bmp")) return strdup("image/bmp");
        if (!strcasecmp(dot, ".tif") || !strcasecmp(dot, ".tiff")) return strdup("image/tiff");
        if (!strcasecmp(dot, ".webp")) return strdup("image/webp");
    }
    return NULL;
}

static void sha256_hex(const unsigned char* data, size_t len, char out_hex[65]) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(digest, &ctx);
    static const char* hex = "0123456789abcdef";
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        out_hex[2*i]   = hex[(digest[i] >> 4) & 0xF];
        out_hex[2*i+1] = hex[digest[i] & 0xF];
    }
    out_hex[64] = '\0';
}

static void ensure_table_exists(MYSQL* conn) {
    const char* ddl =
        "CREATE TABLE IF NOT EXISTS " TABLE_NAME " ("
        "id BIGINT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
        "name VARCHAR(255) NOT NULL,"
        "mime_type VARCHAR(100) NOT NULL,"
        "data LONGBLOB NOT NULL,"
        "sha256 CHAR(64) NOT NULL,"
        "created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "UNIQUE KEY uniq_sha256 (sha256)"
        ") CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci";
    if (mysql_query(conn, ddl) != 0) {
        fprintf(stderr, "DDL error: %s\n", mysql_error(conn));
        exit(1);
    }
}

long long store_image_to_db(const char* host, unsigned int port, const char* user, const char* pass, const char* database, const char* image_path) {
    size_t size = 0;
    if (!file_exists_regular(image_path, &size)) {
        fprintf(stderr, "File not found or not regular: %s\n", image_path);
        exit(1);
    }
    if (size == 0 || size > MAX_FILE_SIZE) {
        fprintf(stderr, "Invalid file size\n");
        exit(1);
    }

    FILE* f = fopen(image_path, "rb");
    if (!f) { perror("fopen"); exit(1); }
    unsigned char head[64] = {0};
    size_t head_len = fread(head, 1, sizeof(head), f);
    fseek(f, 0, SEEK_SET);

    unsigned char* data = (unsigned char*)malloc(size);
    if (!data) { fclose(f); fprintf(stderr, "malloc failed\n"); exit(1); }
    if (fread(data, 1, size, f) != size) { fclose(f); free(data); fprintf(stderr, "read failed\n"); exit(1); }
    fclose(f);

    char* mime = detect_image_mime(head, head_len, image_path);
    if (!mime) {
        free(data);
        fprintf(stderr, "Unsupported or unrecognized image format\n");
        exit(1);
    }

    char sha256[65];
    sha256_hex(data, size, sha256);

    MYSQL* conn = mysql_init(NULL);
    if (!conn) { free(data); free(mime); fprintf(stderr, "mysql_init failed\n"); exit(1); }

    unsigned int timeout = 10;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &timeout);

    if (!mysql_real_connect(conn, host, user, pass, database, port, NULL, 0)) {
        fprintf(stderr, "Connect failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        free(data); free(mime);
        exit(1);
    }

    ensure_table_exists(conn);

    const char* sql = "INSERT INTO " TABLE_NAME " (name, mime_type, data, sha256) VALUES (?, ?, ?, ?)";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init failed\n");
        mysql_close(conn);
        free(data); free(mime);
        exit(1);
    }
    if (mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0) {
        fprintf(stderr, "mysql_stmt_prepare failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        free(data); free(mime);
        exit(1);
    }

    const char* base = strrchr(image_path, '/');
    const char* base2 = strrchr(image_path, '\\');
    const char* fname = image_path;
    if (base && base2) fname = (base > base2) ? base + 1 : base2 + 1;
    else if (base) fname = base + 1;
    else if (base2) fname = base2 + 1;

    MYSQL_BIND bind[4];
    memset(bind, 0, sizeof(bind));

    unsigned long name_len = (unsigned long)strlen(fname);
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)fname;
    bind[0].buffer_length = name_len;
    bind[0].length = &name_len;

    unsigned long mime_len = (unsigned long)strlen(mime);
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)mime;
    bind[1].buffer_length = mime_len;
    bind[1].length = &mime_len;

    unsigned long data_len = (unsigned long)size;
    bind[2].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[2].buffer = (void*)data;
    bind[2].buffer_length = data_len;
    bind[2].length = &data_len;

    unsigned long sha_len = 64;
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (void*)sha256;
    bind[3].buffer_length = sha_len;
    bind[3].length = &sha_len;

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        fprintf(stderr, "mysql_stmt_bind_param failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        free(data); free(mime);
        exit(1);
    }

    if (mysql_stmt_execute(stmt) != 0) {
        fprintf(stderr, "mysql_stmt_execute failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        free(data); free(mime);
        exit(1);
    }

    long long inserted_id = (long long)mysql_stmt_insert_id(stmt);

    mysql_stmt_close(stmt);
    mysql_close(conn);
    free(data);
    free(mime);

    return inserted_id;
}

int main(void) {
    const char* host = getenv("DB_HOST") ? getenv("DB_HOST") : "127.0.0.1";
    unsigned int port = getenv("DB_PORT") ? (unsigned int)strtoul(getenv("DB_PORT"), NULL, 10) : 3306;
    const char* db   = getenv("DB_NAME") ? getenv("DB_NAME") : "test";
    const char* user = getenv("DB_USER") ? getenv("DB_USER") : "root";
    const char* pass = getenv("DB_PASS") ? getenv("DB_PASS") : "";

    const char* tests[5] = {
        "sample1.jpg",
        "sample2.png",
        "not_found_image.bin",
        "test.txt",
        "small.gif"
    };

    for (int i = 0; i < 5; ++i) {
        printf("Running test %d with file %s\n", i+1, tests[i]);
        // In a real environment, handle errors gracefully per test.
        // For demonstration, we catch failures by printing messages and continuing.
        size_t size = 0;
        if (!file_exists_regular(tests[i], &size)) {
            fprintf(stderr, "Test %d: Failed for file %s -> not found or not regular\n", i+1, tests[i]);
            continue;
        }
        if (size == 0 || size > MAX_FILE_SIZE) {
            fprintf(stderr, "Test %d: Failed for file %s -> invalid size\n", i+1, tests[i]);
            continue;
        }
        long long id = store_image_to_db(host, port, user, pass, db, tests[i]);
        printf("Test %d: Inserted ID = %lld for file %s\n", i+1, id, tests[i]);
    }

    return 0;
}