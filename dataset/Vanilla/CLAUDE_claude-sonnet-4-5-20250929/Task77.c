
// Note: C FTP operations require external libraries like libcurl
// This code requires libcurl library to compile and run
// Compile: gcc -o Task77 Task77.c -lcurl

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int downloadFile(const char* hostname, const char* username, const char* password, 
                 const char* remoteFilePath, const char* localFilePath) {
    CURL *curl;
    CURLcode res;
    FILE *fp;
    char url[512];
    char credentials[256];
    
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize CURL\\n");
        return 0;
    }
    
    fp = fopen(localFilePath, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open local file for writing\\n");
        curl_easy_cleanup(curl);
        return 0;
    }
    
    snprintf(url, sizeof(url), "ftp://%s%s", hostname, remoteFilePath);
    snprintf(credentials, sizeof(credentials), "%s:%s", username, password);
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERPWD, credentials);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    
    res = curl_easy_perform(curl);
    
    fclose(fp);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "FTP download failed: %s\\n", curl_easy_strerror(res));
        return 0;
    }
    
    printf("File downloaded successfully: %s\\n", localFilePath);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <hostname> <username> <password> <remote_file_path>\\n", argv[0]);
        printf("\\nRunning test cases with dummy data...\\n\\n");
        
        // Test cases with dummy data
        printf("Test Case 1:\\n");
        downloadFile("ftp.example.com", "user1", "pass1", "/files/data.txt", "data1.txt");
        
        printf("\\nTest Case 2:\\n");
        downloadFile("ftp.test.com", "admin", "admin123", "/documents/report.pdf", "report2.pdf");
        
        printf("\\nTest Case 3:\\n");
        downloadFile("192.168.1.100", "ftpuser", "ftppass", "/backup/backup.zip", "backup3.zip");
        
        printf("\\nTest Case 4:\\n");
        downloadFile("ftp.server.org", "guest", "guest", "/public/readme.txt", "readme4.txt");
        
        printf("\\nTest Case 5:\\n");
        downloadFile("ftp.demo.net", "testuser", "test@123", "/files/config.xml", "config5.xml");
    } else {
        const char* hostname = argv[1];
        const char* username = argv[2];
        const char* password = argv[3];
        const char* remoteFile = (argc > 4) ? argv[4] : "/data.txt";
        
        const char* lastSlash = strrchr(remoteFile, '/');
        const char* localFile = (lastSlash != NULL) ? (lastSlash + 1) : remoteFile;
        
        downloadFile(hostname, username, password, remoteFile, localFile);
    }
    
    return 0;
}
