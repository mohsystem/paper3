
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void httpHelloPage(FILE* request, FILE* response) {
    char username[256] = "";
    char line[1024];
    
    // Read from request to find username parameter
    while (fgets(line, sizeof(line), request) != NULL) {
        char* pos = strstr(line, "username=");
        if (pos != NULL) {
            pos += 9; // Move past "username="
            int i = 0;
            while (pos[i] != '\\0' && pos[i] != '&' && pos[i] != ' ' && pos[i] != '\\n' && pos[i] != '\\r') {
                username[i] = pos[i];
                i++;
            }
            username[i] = '\\0';
            break;
        }
    }
    
    // Construct response
    fprintf(response, "HTTP/1.1 200 OK\\n");
    fprintf(response, "Content-Type: text/html\\n");
    fprintf(response, "\\n");
    fprintf(response, "Hello %s\\n", username);
}

int main() {
    // Test case 1
    FILE* request1 = fopen("test1.txt", "w+");
    FILE* response1 = fopen("response1.txt", "w+");
    fprintf(request1, "GET /hello?username=John HTTP/1.1\\n");
    rewind(request1);
    httpHelloPage(request1, response1);
    rewind(response1);
    printf("Test 1:\\n");
    char line[256];
    while (fgets(line, sizeof(line), response1) != NULL) {
        printf("%s", line);
    }
    printf("\\n");
    fclose(request1);
    fclose(response1);
    
    // Test case 2
    FILE* request2 = fopen("test2.txt", "w+");
    FILE* response2 = fopen("response2.txt", "w+");
    fprintf(request2, "POST /hello\\nusername=Alice");
    rewind(request2);
    httpHelloPage(request2, response2);
    rewind(response2);
    printf("Test 2:\\n");
    while (fgets(line, sizeof(line), response2) != NULL) {
        printf("%s", line);
    }
    printf("\\n");
    fclose(request2);
    fclose(response2);
    
    // Test case 3
    FILE* request3 = fopen("test3.txt", "w+");
    FILE* response3 = fopen("response3.txt", "w+");
    fprintf(request3, "GET /hello?username=Bob&age=25 HTTP/1.1\\n");
    rewind(request3);
    httpHelloPage(request3, response3);
    rewind(response3);
    printf("Test 3:\\n");
    while (fgets(line, sizeof(line), response3) != NULL) {
        printf("%s", line);
    }
    printf("\\n");
    fclose(request3);
    fclose(response3);
    
    // Test case 4
    FILE* request4 = fopen("test4.txt", "w+");
    FILE* response4 = fopen("response4.txt", "w+");
    fprintf(request4, "GET /hello?username=Charlie HTTP/1.1\\n");
    rewind(request4);
    httpHelloPage(request4, response4);
    rewind(response4);
    printf("Test 4:\\n");
    while (fgets(line, sizeof(line), response4) != NULL) {
        printf("%s", line);
    }
    printf("\\n");
    fclose(request4);
    fclose(response4);
    
    // Test case 5
    FILE* request5 = fopen("test5.txt", "w+");
    FILE* response5 = fopen("response5.txt", "w+");
    fprintf(request5, "POST /hello\\nusername=Eve&password=secret");
    rewind(request5);
    httpHelloPage(request5, response5);
    rewind(response5);
    printf("Test 5:\\n");
    while (fgets(line, sizeof(line), response5) != NULL) {
        printf("%s", line);
    }
    printf("\\n");
    fclose(request5);
    fclose(response5);
    
    return 0;
}
