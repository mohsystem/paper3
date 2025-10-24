
// Note: C++ FTP operations require external libraries like libcurl
// This code requires libcurl library to compile and run
// Compile: g++ -o Task77 Task77.cpp -lcurl

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <curl/curl.h>

using namespace std;

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

bool downloadFile(const string& hostname, const string& username, const string& password, 
                  const string& remoteFilePath, const string& localFilePath) {
    CURL *curl;
    CURLcode res;
    FILE *fp;
    
    curl = curl_easy_init();
    if (!curl) {
        cerr << "Failed to initialize CURL" << endl;
        return false;
    }
    
    fp = fopen(localFilePath.c_str(), "wb");
    if (!fp) {
        cerr << "Failed to open local file for writing" << endl;
        curl_easy_cleanup(curl);
        return false;
    }
    
    string url = "ftp://" + hostname + remoteFilePath;
    string credentials = username + ":" + password;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERPWD, credentials.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    
    res = curl_easy_perform(curl);
    
    fclose(fp);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        cerr << "FTP download failed: " << curl_easy_strerror(res) << endl;
        return false;
    }
    
    cout << "File downloaded successfully: " << localFilePath << endl;
    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        cout << "Usage: " << argv[0] << " <hostname> <username> <password> <remote_file_path>" << endl;
        cout << "\\nRunning test cases with dummy data...\\n" << endl;
        
        // Test cases with dummy data
        cout << "Test Case 1:" << endl;
        downloadFile("ftp.example.com", "user1", "pass1", "/files/data.txt", "data1.txt");
        
        cout << "\\nTest Case 2:" << endl;
        downloadFile("ftp.test.com", "admin", "admin123", "/documents/report.pdf", "report2.pdf");
        
        cout << "\\nTest Case 3:" << endl;
        downloadFile("192.168.1.100", "ftpuser", "ftppass", "/backup/backup.zip", "backup3.zip");
        
        cout << "\\nTest Case 4:" << endl;
        downloadFile("ftp.server.org", "guest", "guest", "/public/readme.txt", "readme4.txt");
        
        cout << "\\nTest Case 5:" << endl;
        downloadFile("ftp.demo.net", "testuser", "test@123", "/files/config.xml", "config5.xml");
    } else {
        string hostname = argv[1];
        string username = argv[2];
        string password = argv[3];
        string remoteFile = (argc > 4) ? argv[4] : "/data.txt";
        string localFile = remoteFile.substr(remoteFile.find_last_of("/") + 1);
        
        downloadFile(hostname, username, password, remoteFile, localFile);
    }
    
    return 0;
}
