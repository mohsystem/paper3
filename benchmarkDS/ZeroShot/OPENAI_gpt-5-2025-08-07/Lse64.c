#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
#else
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#endif

#include <mysql/mysql.h>

#define MAX_SIZE (10 * 1024 * 1024)
#define UPLOADS_DIR "uploads"

typedef struct {
    const char* host;
    const char* user;
    const char* password;
    const char* database;
    unsigned int port;
} DBConfig;

static int ensure_uploads_dir(char* out_real, size_t out_sz) {
#ifdef _WIN32
    _mkdir(UPLOADS_DIR);
#else
    mkdir(UPLOADS_DIR, 0700);
#endif
    char tmp[PATH_MAX];
    if (!realpath(UPLOADS_DIR, tmp)) {
        return -1;
    }
    strncpy(out_real, tmp, out_sz - 1);
    out_real[out_sz - 1] = '\0';
    return 0;
}

static void sanitize_filename(const char* original, char* out, size_t out_sz) {
    const char* name = original ? original : "file.pdf";
    const char* p = name + strlen(name);
    while (p > name && (*(p-1) == '/' || *(p-1) == '\\')) p--;
    const char* start = name;
    for (const char* q = name; *q; ++q) {
        if (*q == '/' || *q == '\\') start = q + 1;
    }
    size_t n = 0;
    for (const char* q = start; *q && n + 1 < out_sz; ++q) {
        unsigned char ch = (unsigned char)*q;
        if (isalnum(ch) || ch == '.' || ch == '_' || ch == '-') {
            out[n++] = (char)ch;
        } else {
            out[n++] = '_';
        }
    }
    out[n] = '\0';
    size_t len = strlen(out);
    if (len < 4 || strcasecmp(out + len - 4, ".pdf") != 0) {
        if (n + 4 < out_sz) {
            strcat(out, ".pdf");
        }
    }
    len = strlen(out);
    if (len > 100) {
        memmove(out, out + (len - 100), 100);
        out[100] = '\0';
    }
}

static int is_valid_pdf(const unsigned char* data, size_t len) {
    if (!data || len == 0 || len > MAX_SIZE) return 0;
    if (len < 5) return 0;
    if (!(data[0] == '%' && data[1] == 'P' && data[2] == 'D' && data[3] == 'F' && data[4] == '-')) return 0;
    const char* eof = "%%EOF";
    for (size_t i = 0; i + 5 <= len; ++i) {
        if (memcmp(data + i, eof, 5) == 0) return 1;
    }
    return 0;
}

static void save_to_db(const DBConfig* cfg, const char* rel_path) {
    if (!cfg) return;
    MYSQL* conn = mysql_init(NULL);
    if (!conn) return;
    if (!mysql_real_connect(conn, cfg->host, cfg->user, cfg->password, cfg->database, cfg->port, NULL, 0)) {
        mysql_close(conn);
        return;
    }
    const char* createSql =
        "CREATE TABLE IF NOT EXISTS documents ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "path VARCHAR(1024) NOT NULL,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";
    if (mysql_query(conn, createSql) != 0) {
        mysql_close(conn);
        return;
    }
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        mysql_close(conn);
        return;
    }
    const char* insertSql = "INSERT INTO documents (path) VALUES (?)";
    if (mysql_stmt_prepare(stmt, insertSql, (unsigned long)strlen(insertSql)) != 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }
    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));
    unsigned long len = (unsigned long)strlen(rel_path);
    bind.buffer_type = MYSQL_TYPE_STRING;
    bind.buffer = (void*)rel_path;
    bind.buffer_length = len;
    bind.length = &len;
    if (mysql_stmt_bind_param(stmt, &bind) != 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }
    mysql_stmt_close(stmt);
    mysql_close(conn);
}

char* upload_pdf(const unsigned char* data, size_t len, const char* original_filename, const DBConfig* cfg) {
    if (!is_valid_pdf(data, len)) {
        return NULL;
    }

    char uploads_real[PATH_MAX];
    if (ensure_uploads_dir(uploads_real, sizeof(uploads_real)) != 0) {
        return NULL;
    }

    char safe[256];
    sanitize_filename(original_filename, safe, sizeof(safe));
    char base[256];
    strncpy(base, safe, sizeof(base)-1);
    base[sizeof(base)-1] = '\0';
    size_t blen = strlen(base);
    if (blen >= 4 && strcasecmp(base + blen - 4, ".pdf") == 0) {
        base[blen - 4] = '\0';
    }

    char final_path[PATH_MAX];
    int attempts = 0;
    for (;;) {
        char unique[33];
        for (int i = 0; i < 32; ++i) {
            int r = rand() % 16;
            unique[i] = "0123456789abcdef"[r];
        }
        unique[32] = '\0';
        snprintf(final_path, sizeof(final_path), "%s/%s_%s.pdf", uploads_real, base, unique);
        FILE* fp = fopen(final_path, "rb");
        if (fp) {
            fclose(fp);
            attempts++;
            if (attempts > 10) return NULL;
            continue;
        }
        break;
    }

#ifdef _WIN32
    FILE* f = fopen(final_path, "wb");
    if (!f) return NULL;
    size_t written = fwrite(data, 1, len, f);
    fclose(f);
    if (written != len) {
        remove(final_path);
        return NULL;
    }
#else
    int fd = open(final_path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) return NULL;
    ssize_t w = write(fd, data, len);
    close(fd);
    if (w < 0 || (size_t)w != len) {
        remove(final_path);
        return NULL;
    }
    chmod(final_path, 0600);
#endif

    char target_real[PATH_MAX];
    if (!realpath(final_path, target_real)) {
        remove(final_path);
        return NULL;
    }
    size_t upl_len = strlen(uploads_real);
    if (strncmp(target_real, uploads_real, upl_len) != 0 ||
        (target_real[upl_len] != '/' && target_real[upl_len] != '\0')) {
        remove(target_real);
        return NULL;
    }

    const char* rel = target_real + upl_len;
    if (*rel == '/' || *rel == '\\') rel++;
    char* rel_copy = (char*)malloc(strlen(rel) + 1);
    if (!rel_copy) return NULL;
    strcpy(rel_copy, rel);

    for (char* p = rel_copy; *p; ++p) {
        if (*p == '\\') *p = '/';
    }

    save_to_db(cfg, rel_copy);
    return rel_copy; // caller must free
}

unsigned char* download_pdf(const char* stored_path, size_t* out_len) {
    if (!stored_path || !out_len) return NULL;
    char uploads_real[PATH_MAX];
    if (ensure_uploads_dir(uploads_real, sizeof(uploads_real)) != 0) return NULL;

    char combined[PATH_MAX];
    snprintf(combined, sizeof(combined), "%s/%s", uploads_real, stored_path);

    char target_real[PATH_MAX];
    if (!realpath(combined, target_real)) return NULL;

    size_t upl_len = strlen(uploads_real);
    if (strncmp(target_real, uploads_real, upl_len) != 0 ||
        (target_real[upl_len] != '/' && target_real[upl_len] != '\0')) {
        return NULL;
    }

    FILE* f = fopen(target_real, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if (sz < 0 || sz > MAX_SIZE) { fclose(f); return NULL; }
    rewind(f);
    unsigned char* buf = (unsigned char*)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (rd != (size_t)sz) { free(buf); return NULL; }
    *out_len = (size_t)sz;
    return buf;
}

static DBConfig* cfg_from_env() {
    const char* host = getenv("DB_HOST");
    const char* user = getenv("DB_USER");
    const char* pass = getenv("DB_PASS");
    const char* name = getenv("DB_NAME");
    const char* portStr = getenv("DB_PORT");
    if (!host || !user || !pass || !name) return NULL;
    DBConfig* cfg = (DBConfig*)malloc(sizeof(DBConfig));
    cfg->host = host;
    cfg->user = user;
    cfg->password = pass;
    cfg->database = name;
    cfg->port = portStr ? (unsigned int)strtoul(portStr, NULL, 10) : 3306;
    return cfg;
}

int main() {
    DBConfig* cfg = cfg_from_env();
    const char* filenames[5] = {"report.pdf","invoice..pdf","../../evil.pdf","résumé.pdf","noext"};
    unsigned char* pdfs[5];
    size_t lens[5];
    for (int i = 0; i < 5; ++i) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%%PDF-1.4\n%% test %d\n1 0 obj\n<<>>\nendobj\n%%%%EOF\n", i);
        size_t l = strlen(buf);
        pdfs[i] = (unsigned char*)malloc(l);
        memcpy(pdfs[i], buf, l);
        lens[i] = l;
    }
    char* paths[5] = {0};
    for (int i = 0; i < 5; ++i) {
        paths[i] = upload_pdf(pdfs[i], lens[i], filenames[i], cfg);
        if (paths[i]) {
            printf("Uploaded: %s\n", paths[i]);
        } else {
            printf("Upload failed %d\n", i);
        }
    }
    for (int i = 0; i < 5; ++i) {
        if (!paths[i]) {
            printf("Skip download %d\n", i);
            continue;
        }
        size_t out_len = 0;
        unsigned char* got = download_pdf(paths[i], &out_len);
        if (!got) {
            printf("Download failed %d\n", i);
        } else {
            int ok = (out_len == lens[i] && memcmp(got, pdfs[i], lens[i]) == 0);
            printf("Download %d ok=%s\n", i, ok ? "true" : "false");
            free(got);
        }
    }
    for (int i = 0; i < 5; ++i) {
        free(pdfs[i]);
        if (paths[i]) free(paths[i]);
    }
    if (cfg) free(cfg);
    return 0;
}