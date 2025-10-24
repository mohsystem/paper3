
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

/* Maximum file size: 100MB to prevent resource exhaustion */
#define MAX_FILE_SIZE (100 * 1024 * 1024)
/* Maximum URL length to prevent buffer issues */
#define MAX_URL_LENGTH 2048
/* Maximum path length */
#define MAX_PATH_LENGTH 512

/* Secure memory cleanup - explicit zeroing that compiler cannot optimize away */
static void secure_zero(void *ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) *p++ = 0;
}

/* Structure to hold download data with size limit enforcement */
struct download_buffer {
    unsigned char *data;
    size_t size;
    size_t capacity;
};

/* Write callback for curl - enforces size limits and validates data */
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = 0;
    struct download_buffer *buf = (struct download_buffer *)userp;
    
    /* Validate inputs */
    if (contents == NULL || userp == NULL || size == 0 || nmemb == 0) {
        return 0;
    }
    
    /* Check for integer overflow in multiplication */
    if (nmemb > SIZE_MAX / size) {
        fprintf(stderr, "Error: Size calculation overflow\\n");
        return 0;
    }
    realsize = size * nmemb;
    
    /* Check if adding new data would exceed max file size */
    if (buf->size > MAX_FILE_SIZE || realsize > MAX_FILE_SIZE - buf->size) {
        fprintf(stderr, "Error: File size exceeds maximum allowed (%d bytes)\\n", MAX_FILE_SIZE);
        return 0;
    }
    
    /* Grow buffer if needed */
    if (buf->size + realsize > buf->capacity) {
        size_t new_capacity = buf->capacity * 2;
        if (new_capacity < buf->size + realsize) {
            new_capacity = buf->size + realsize;
        }
        /* Cap at maximum file size */
        if (new_capacity > MAX_FILE_SIZE) {
            new_capacity = MAX_FILE_SIZE;
        }
        
        unsigned char *new_data = (unsigned char *)realloc(buf->data, new_capacity);
        if (new_data == NULL) {
            fprintf(stderr, "Error: Memory allocation failed\\n");
            return 0;
        }
        buf->data = new_data;
        buf->capacity = new_capacity;
    }
    
    /* Copy data with bounds check */
    memcpy(buf->data + buf->size, contents, realsize);
    buf->size += realsize;
    
    return realsize;
}

/* Validate and sanitize filename to prevent path traversal */
static int validate_filename(const char *filename, char *safe_name, size_t safe_name_size) {
    size_t len = 0;
    size_t i = 0;
    
    if (filename == NULL || safe_name == NULL || safe_name_size == 0) {
        return -1;
    }
    
    len = strlen(filename);
    if (len == 0 || len >= safe_name_size) {
        return -1;
    }
    
    /* Reject paths with directory traversal attempts */
    if (strstr(filename, "..") != NULL || 
        strstr(filename, "/") != NULL || 
        strstr(filename, "\\\\") != NULL) {\n        fprintf(stderr, "Error: Invalid filename - path traversal detected\
");\n        return -1;\n    }\n    \n    /* Copy only safe characters: alphanumeric, dash, underscore, dot */\n    for (i = 0; i < len && i < safe_name_size - 1; i++) {\n        char c = filename[i];\n        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || \n            (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.') {\n            safe_name[i] = c;\n        } else {\n            fprintf(stderr, "Error: Invalid character in filename\
");\n            return -1;\n        }\n    }\n    safe_name[i] = '\\0';\n    \n    /* Reject empty result or files starting with dot (hidden files) */\n    if (safe_name[0] == '\\0' || safe_name[0] == '.') {\n        fprintf(stderr, "Error: Invalid filename\
");\n        return -1;\n    }\n    \n    return 0;\n}\n\n/* Securely write file using open-validate-write pattern to avoid TOCTOU */\nstatic int secure_write_file(const char *filename, const unsigned char *data, size_t size) {\n    int fd = -1;\n    ssize_t written = 0;\n    struct stat st;\n    char temp_name[MAX_PATH_LENGTH];\n    int ret = -1;\n    \n    if (filename == NULL || data == NULL || size == 0) {\n        return -1;\n    }\n    \n    /* Create temporary file name */\n    ret = snprintf(temp_name, sizeof(temp_name), ".%s.tmp.XXXXXX", filename);\n    if (ret < 0 || (size_t)ret >= sizeof(temp_name)) {\n        fprintf(stderr, "Error: Filename too long\
");\n        return -1;\n    }\n    \n    /* Create temporary file with restrictive permissions (0600) and O_EXCL to prevent race */\n    fd = mkstemp(temp_name);\n    if (fd < 0) {\n        fprintf(stderr, "Error: Cannot create temporary file: %s\
", strerror(errno));\n        return -1;\n    }\n    \n    /* Validate the opened file descriptor */\n    if (fstat(fd, &st) != 0) {\n        fprintf(stderr, "Error: Cannot stat temporary file\
");\n        close(fd);\n        unlink(temp_name);\n        return -1;\n    }\n    \n    /* Ensure it's a regular file */\n    if (!S_ISREG(st.st_mode)) {\n        fprintf(stderr, "Error: Not a regular file\
");\n        close(fd);\n        unlink(temp_name);\n        return -1;\n    }\n    \n    /* Write data in chunks with error checking */\n    size_t total_written = 0;\n    while (total_written < size) {\n        written = write(fd, data + total_written, size - total_written);\n        if (written < 0) {\n            if (errno == EINTR) continue;\n            fprintf(stderr, "Error: Write failed: %s\
", strerror(errno));\n            close(fd);\n            unlink(temp_name);\n            return -1;\n        }\n        total_written += (size_t)written;\n    }\n    \n    /* Flush to disk */\n    if (fsync(fd) != 0) {\n        fprintf(stderr, "Error: fsync failed: %s\
", strerror(errno));\n        close(fd);\n        unlink(temp_name);\n        return -1;\n    }\n    \n    close(fd);\n    \n    /* Atomic rename to target filename */\n    if (rename(temp_name, filename) != 0) {\n        fprintf(stderr, "Error: Cannot rename file: %s\
", strerror(errno));\n        unlink(temp_name);\n        return -1;\n    }\n    \n    /* Set final restrictive permissions */\n    if (chmod(filename, 0600) != 0) {\n        fprintf(stderr, "Warning: Cannot set file permissions\
");\n    }\n    \n    return 0;\n}\n\n/* Download file from FTP server with full security controls */\nstatic int download_ftp_file(const char *hostname, const char *username, \n                             const char *password, const char *remote_file, \n                             const char *local_file) {\n    CURL *curl = NULL;\n    CURLcode res = CURLE_OK;\n    char url[MAX_URL_LENGTH];\n    char *encoded_file = NULL;\n    struct download_buffer buffer = {NULL, 0, 0};\n    int ret = -1;\n    int snprintf_ret = 0;\n    \n    /* Validate all inputs - treat as untrusted */\n    if (hostname == NULL || username == NULL || password == NULL || \n        remote_file == NULL || local_file == NULL) {\n        fprintf(stderr, "Error: Invalid input parameters\
");\n        return -1;\n    }\n    \n    if (strlen(hostname) == 0 || strlen(remote_file) == 0 || strlen(local_file) == 0) {\n        fprintf(stderr, "Error: Empty input parameters\
");\n        return -1;\n    }\n    \n    /* Initialize buffer with initial capacity */\n    buffer.capacity = 4096;\n    buffer.data = (unsigned char *)malloc(buffer.capacity);\n    if (buffer.data == NULL) {\n        fprintf(stderr, "Error: Memory allocation failed\
");\n        return -1;\n    }\n    buffer.size = 0;\n    \n    /* Initialize curl */\n    curl = curl_easy_init();\n    if (curl == NULL) {\n        fprintf(stderr, "Error: curl initialization failed\
");\n        free(buffer.data);\n        return -1;\n    }\n    \n    /* URL encode the remote filename to handle special characters safely */\n    encoded_file = curl_easy_escape(curl, remote_file, 0);\n    if (encoded_file == NULL) {\n        fprintf(stderr, "Error: Failed to encode filename\
");\n        curl_easy_cleanup(curl);\n        free(buffer.data);\n        return -1;\n    }\n    \n    /* Construct FTP URL - validate length to prevent overflow */\n    snprintf_ret = snprintf(url, sizeof(url), "ftp://%s/%s", hostname, encoded_file);\n    curl_free(encoded_file);\n    \n    if (snprintf_ret < 0 || (size_t)snprintf_ret >= sizeof(url)) {\n        fprintf(stderr, "Error: URL too long\
");\n        curl_easy_cleanup(curl);\n        free(buffer.data);\n        return -1;\n    }\n    \n    /* Configure curl with secure options */\n    \n    /* Set the URL - restrict to FTP protocol only */\n    curl_easy_setopt(curl, CURLOPT_URL, url);\n    \n    /* Restrict protocols to FTP only - no other protocols allowed */\n    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_FTP);\n    \n    /* Set credentials - these should ideally come from secure storage, not command line */\n    curl_easy_setopt(curl, CURLOPT_USERNAME, username);\n    curl_easy_setopt(curl, CURLOPT_PASSWORD, password);\n    \n    /* Enable TLS for FTPS if server supports it */\n    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);\n    \n    /* Enforce certificate verification for FTPS connections */\n    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);\n    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);\n    \n    /* Set minimum TLS version to 1.2 for FTPS */\n    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);\n    \n    /* Set write callback to capture data with size limits */\n    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);\n    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);\n    \n    /* Set timeouts to prevent hanging */\n    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);\n    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);\n    \n    /* Limit redirects */\n    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 0L);\n    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);\n    \n    /* Disable progress meter but enable failure on error */\n    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);\n    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);\n    \n    /* Perform the download */\n    res = curl_easy_perform(curl);\n    \n    if (res != CURLE_OK) {\n        fprintf(stderr, "Error: Download failed: %s\
", curl_easy_strerror(res));\n        curl_easy_cleanup(curl);\n        secure_zero(buffer.data, buffer.size);\n        free(buffer.data);\n        return -1;\n    }\n    \n    /* Verify we received data */\n    if (buffer.size == 0) {\n        fprintf(stderr, "Error: No data received\
");\n        curl_easy_cleanup(curl);\n        free(buffer.data);\n        return -1;\n    }\n    \n    /* Write to file securely */\n    ret = secure_write_file(local_file, buffer.data, buffer.size);\n    \n    /* Clean up */\n    curl_easy_cleanup(curl);\n    \n    /* Securely clear sensitive data from memory */\n    secure_zero(buffer.data, buffer.size);\n    free(buffer.data);\n    \n    if (ret == 0) {\n        printf("File downloaded successfully to: %s\
", local_file);\n        return 0;\n    }\n    \n    return -1;\n}\n\nint main(int argc, char *argv[]) {\n    char safe_remote_file[MAX_PATH_LENGTH];\n    char safe_local_file[MAX_PATH_LENGTH];\n    int result = 0;\n    \n    /* Validate argument count */\n    if (argc != 5) {\n        fprintf(stderr, "Usage: %s <hostname> <username> <password> <remote_file>\
", \n                argv[0] ? argv[0] : "program");\n        fprintf(stderr, "Warning: Passing passwords via command line is insecure.\
");\n        fprintf(stderr, "         Use environment variables or secure credential storage instead.\
");\n        return 1;\n    }\n    \n    /* Validate and sanitize remote filename */\n    if (validate_filename(argv[4], safe_remote_file, sizeof(safe_remote_file)) != 0) {\n        fprintf(stderr, "Error: Invalid remote filename\
");\n        return 1;\n    }\n    \n    /* Use sanitized filename as local filename */\n    if (snprintf(safe_local_file, sizeof(safe_local_file), "%s", safe_remote_file) < 0 ||\n        safe_local_file[0] == '\\0') {\n        fprintf(stderr, "Error: Invalid local filename\
");
        return 1;
    }
    
    /* Initialize curl globally */
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    /* Perform download with security controls */
    result = download_ftp_file(argv[1], argv[2], argv[3], safe_remote_file, safe_local_file);
    
    /* Securely clear password from memory */
    if (argv[3] != NULL) {
        secure_zero(argv[3], strlen(argv[3]));
    }
    
    /* Cleanup curl */
    curl_global_cleanup();
    
    return (result == 0) ? 0 : 1;
}
