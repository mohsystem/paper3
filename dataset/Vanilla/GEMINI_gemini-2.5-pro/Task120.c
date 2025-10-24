#include <stdio.h>
#include <stdlib.h>

/**
 * Creates an HTML file with the given content.
 *
 * @param content The user-supplied string to display on the web page.
 * @param filename The name of the HTML file to create.
 */
void createWebPage(const char* content, const char* filename) {
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    fprintf(file, "<!DOCTYPE html>\n");
    fprintf(file, "<html lang=\"en\">\n");
    fprintf(file, "<head>\n");
    fprintf(file, "    <meta charset=\"UTF-8\">\n");
    fprintf(file, "    <title>User Input Display</title>\n");
    fprintf(file, "</head>\n");
    fprintf(file, "<body>\n");
    fprintf(file, "    <h1>User-Supplied Content:</h1>\n");
    fprintf(file, "    <p>%s</p>\n", content);
    fprintf(file, "</body>\n");
    fprintf(file, "</html>\n");

    fclose(file);
    printf("Successfully created %s\n", filename);
}

int main() {
    // 5 Test Cases
    createWebPage("Hello, World!", "c_test1.html");
    createWebPage("This is a test from C.", "c_test2.html");
    createWebPage("12345", "c_test3.html");
    createWebPage("Special characters: <script>alert('xss')</script>", "c_test4.html");
    createWebPage("A longer sentence to see how it wraps inside the paragraph tag on the web page.", "c_test5.html");
    
    return 0;
}