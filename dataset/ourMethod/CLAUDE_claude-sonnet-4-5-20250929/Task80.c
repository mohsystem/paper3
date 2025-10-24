
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

// Maximum hostname length (Rules#1: validate lengths)
#define MAX_HOSTNAME_LEN 255

// Structure to return both SSL and socket descriptor
typedef struct {
    SSL* ssl;
    int sockfd;
} SSLConnection;

// Global SSL context (initialized once)
static SSL_CTX* g_ssl_ctx = NULL;

// Initialize SSL context once (Rules#6: use TLS 1.2+)
int initSSLContext(void) {
    if (g_ssl_ctx != NULL) {
        return 1; // Already initialized
    }
    
    // Initialize OpenSSL library
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    
    // Create SSL context with TLS 1.2+ (Rules#6)
    const SSL_METHOD* method = TLS_client_method();
    if (method == NULL) {
        fprintf(stderr, "Failed to get TLS method\\n");
        return 0;
    }
    
    g_ssl_ctx = SSL_CTX_new(method);
    if (g_ssl_ctx == NULL) {
        fprintf(stderr, "Failed to create SSL context\\n");
        return 0;
    }
    
    // Set minimum TLS version to 1.2 (Rules#6: enforce TLS 1.2+)
    if (SSL_CTX_set_min_proto_version(g_ssl_ctx, TLS1_2_VERSION) != 1) {
        SSL_CTX_free(g_ssl_ctx);
        g_ssl_ctx = NULL;
        fprintf(stderr, "Failed to set minimum TLS version\\n");
        return 0;
    }
    
    // Enable certificate verification (Rules#3: certificate validation)
    SSL_CTX_set_verify(g_ssl_ctx, SSL_VERIFY_PEER, NULL);
    
    // Load default CA certificates (Rules#3)
    if (SSL_CTX_set_default_verify_paths(g_ssl_ctx) != 1) {
        SSL_CTX_free(g_ssl_ctx);
        g_ssl_ctx = NULL;
        fprintf(stderr, "Failed to load CA certificates\\n");
        return 0;
    }
    
    return 1;
}

// Validate input parameters (Rules#1, Rules#10: input validation)
int validateInputs(const char* host, int port) {
    size_t host_len;
    
    // Check for NULL pointer (Rules#1: validate all inputs)
    if (host == NULL) {
        fprintf(stderr, "Invalid input: host is NULL\\n");
        return 0;
    }
    
    // Validate hostname length (Rules#1: validate lengths)
    host_len = strlen(host);
    if (host_len == 0 || host_len > MAX_HOSTNAME_LEN) {
        fprintf(stderr, "Invalid hostname length: %zu (must be 1-%d)\\n", 
                host_len, MAX_HOSTNAME_LEN);
        return 0;
    }
    
    // Validate port range (Rules#1: validate ranges)
    if (port < 1 || port > 65535) {
        fprintf(stderr, "Invalid port: %d (must be 1-65535)\\n", port);
        return 0;
    }
    
    return 1;
}

// Create and connect SSL client socket (Rules#3, Rules#4, Rules#6)
SSLConnection* createSSLClientSocket(const char* host, int port) {
    SSLConnection* conn = NULL;
    int sockfd = -1;
    SSL* ssl = NULL;
    struct addrinfo hints, *result = NULL, *rp = NULL;
    char port_str[8];
    int ret;
    long verify_result;
    
    // Validate inputs (Rules#1, Rules#10)
    if (!validateInputs(host, port)) {
        return NULL;
    }
    
    // Initialize SSL context if not already done (Rules#6)
    if (!initSSLContext()) {
        return NULL;
    }
    
    // Allocate connection structure (Rules#1: check malloc return)
    conn = (SSLConnection*)calloc(1, sizeof(SSLConnection));
    if (conn == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    conn->ssl = NULL;
    conn->sockfd = -1;
    
    // Convert port to string safely (Rules#1: safe formatting)
    ret = snprintf(port_str, sizeof(port_str), "%d", port);
    if (ret < 0 || ret >= (int)sizeof(port_str)) {
        fprintf(stderr, "Port string conversion failed\\n");
        free(conn);
        return NULL;
    }
    
    // Setup hints for getaddrinfo (IPv6, TCP only)
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;       // IPv6 only
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_protocol = IPPROTO_TCP;
    
    // Resolve hostname (Rules#1: safe hostname resolution)
    ret = getaddrinfo(host, port_str, &hints, &result);
    if (ret != 0) {
        fprintf(stderr, "Failed to resolve hostname: %s\\n", gai_strerror(ret));
        free(conn);
        return NULL;
    }
    
    // Try each address until successful connection (Rules#1: check return values)
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) {
            continue;
        }
        
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break; // Success
        }
        
        close(sockfd);
        sockfd = -1;
    }
    
    freeaddrinfo(result);
    
    if (sockfd == -1) {
        fprintf(stderr, "Failed to connect to server\\n");
        free(conn);
        return NULL;
    }
    
    // Create SSL object (Rules#1: check return values)
    ssl = SSL_new(g_ssl_ctx);
    if (ssl == NULL) {
        fprintf(stderr, "Failed to create SSL object\\n");
        close(sockfd);
        free(conn);
        return NULL;
    }
    
    // Set socket file descriptor (Rules#1: check return values)
    if (SSL_set_fd(ssl, sockfd) != 1) {
        fprintf(stderr, "Failed to set socket to SSL\\n");
        SSL_free(ssl);
        close(sockfd);
        free(conn);
        return NULL;
    }
    
    // Enable hostname verification (Rules#4: hostname verification)
    SSL_set_hostflags(ssl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    if (SSL_set1_host(ssl, host) != 1) {
        fprintf(stderr, "Failed to set hostname for verification\\n");
        SSL_free(ssl);
        close(sockfd);
        free(conn);
        return NULL;
    }
    
    // Perform SSL handshake (Rules#3: establish secure connection)
    if (SSL_connect(ssl) != 1) {
        unsigned long err = ERR_get_error();
        char err_buf[256];
        ERR_error_string_n(err, err_buf, sizeof(err_buf));
        fprintf(stderr, "SSL handshake failed: %s\\n", err_buf);
        SSL_free(ssl);
        close(sockfd);
        free(conn);
        return NULL;
    }
    
    // Verify server certificate (Rules#3: certificate verification)
    verify_result = SSL_get_verify_result(ssl);
    if (verify_result != X509_V_OK) {
        fprintf(stderr, "Certificate verification failed: %s\\n",
                X509_verify_cert_error_string(verify_result));
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(sockfd);
        free(conn);
        return NULL;
    }
    
    // Set connection members
    conn->ssl = ssl;
    conn->sockfd = sockfd;
    
    return conn;
}

// Clean up SSL connection (Rules#1: proper resource cleanup)
void closeSSLConnection(SSLConnection* conn) {
    if (conn == NULL) {
        return;
    }
    
    if (conn->ssl != NULL) {
        SSL_shutdown(conn->ssl);
        SSL_free(conn->ssl);
        conn->ssl = NULL;
    }
    
    if (conn->sockfd >= 0) {
        close(conn->sockfd);
        conn->sockfd = -1;
    }
    
    free(conn);
}

// Cleanup global SSL context
void cleanupSSL(void) {
    if (g_ssl_ctx != NULL) {
        SSL_CTX_free(g_ssl_ctx);
        g_ssl_ctx = NULL;
    }
}

int main(void) {
    SSLConnection* conn = NULL;
    
    // Test case 1: Valid connection attempt
    printf("Test 1: Attempting connection to ipv6.google.com:443\\n");
    conn = createSSLClientSocket("ipv6.google.com", 443);
    if (conn != NULL) {
        printf("Test 1: SUCCESS - Connected\\n");
        closeSSLConnection(conn);
    } else {
        printf("Test 1: EXPECTED FAILURE (no IPv6 or network)\\n");
    }
    
    // Test case 2: Invalid port (too low)
    printf("\\nTest 2: Invalid port 0\\n");
    conn = createSSLClientSocket("example.com", 0);
    if (conn == NULL) {
        printf("Test 2: SUCCESS - Rejected invalid port\\n");
    } else {
        printf("Test 2: UNEXPECTED SUCCESS\\n");
        closeSSLConnection(conn);
    }
    
    // Test case 3: Invalid port (too high)
    printf("\\nTest 3: Invalid port 70000\\n");
    conn = createSSLClientSocket("example.com", 70000);
    if (conn == NULL) {
        printf("Test 3: SUCCESS - Rejected invalid port\\n");
    } else {
        printf("Test 3: UNEXPECTED SUCCESS\\n");
        closeSSLConnection(conn);
    }
    
    // Test case 4: NULL hostname
    printf("\\nTest 4: NULL hostname\\n");
    conn = createSSLClientSocket(NULL, 443);
    if (conn == NULL) {
        printf("Test 4: SUCCESS - Rejected NULL hostname\\n");
    } else {
        printf("Test 4: UNEXPECTED SUCCESS\\n");
        closeSSLConnection(conn);
    }
    
    // Test case 5: Empty hostname
    printf("\\nTest 5: Empty hostname\\n");
    conn = createSSLClientSocket("", 443);
    if (conn == NULL) {
        printf("Test 5: SUCCESS - Rejected empty hostname\\n");
    } else {
        printf("Test 5: UNEXPECTED SUCCESS\\n");
        closeSSLConnection(conn);
    }
    
    // Cleanup
    cleanupSSL();
    
    return 0;
}
