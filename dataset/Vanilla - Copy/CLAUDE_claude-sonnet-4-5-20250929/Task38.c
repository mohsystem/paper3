
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Note: C is not typically used for web applications
// This is a simulation showing the core logic

typedef struct {
    char name[100];
    char email[100];
    char message[500];
} UserInput;

void escapeHtml(const char* input, char* output, int maxLen) {
    int j = 0;
    for (int i = 0; input[i] != '\\0' && j < maxLen - 10; i++) {
        switch (input[i]) {
            case '&':
                strcpy(&output[j], "&amp;");
                j += 5;
                break;
            case '<':
                strcpy(&output[j], "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(&output[j], "&gt;");
                j += 4;
                break;
            case '"':
                strcpy(&output[j], "&quot;");
                j += 6;
                break;
            case '\\'':
                strcpy(&output[j], "&#x27;");
                j += 6;
                break;
            default:
                output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

void getFormPage(char* output, int maxLen) {
    snprintf(output, maxLen,
        "<!DOCTYPE html>\\n"
        "<html>\\n"
        "<head><title>User Input Form</title></head>\\n"
        "<body>\\n"
        "<h2>Enter Your Information</h2>\\n"
        "<form method='post' action='/submit'>\\n"
        "Name: <input type='text' name='name' required><br><br>\\n"
        "Email: <input type='email' name='email' required><br><br>\\n"
        "Message: <textarea name='message' rows='4' cols='50' required></textarea><br><br>\\n"
        "<input type='submit' value='Submit'>\\n"
        "</form>\\n"
        "</body>\\n"
        "</html>");
}

void getDisplayPage(UserInput* input, char* output, int maxLen) {
    char escapedName[200], escapedEmail[200], escapedMessage[1000];
    
    escapeHtml(input->name, escapedName, 200);
    escapeHtml(input->email, escapedEmail, 200);
    escapeHtml(input->message, escapedMessage, 1000);
    
    snprintf(output, maxLen,
        "<!DOCTYPE html>\\n"
        "<html>\\n"
        "<head><title>Display User Input</title></head>\\n"
        "<body>\\n"
        "<h2>Your Submitted Information</h2>\\n"
        "<p><strong>Name:</strong> %s</p>\\n"
        "<p><strong>Email:</strong> %s</p>\\n"
        "<p><strong>Message:</strong> %s</p>\\n"
        "<br><a href='/'>Go Back</a>\\n"
        "</body>\\n"
        "</html>",
        escapedName, escapedEmail, escapedMessage);
}

void processInput(UserInput* input) {
    printf("Name: %s\\n", input->name);
    printf("Email: %s\\n", input->email);
    printf("Message: %s\\n", input->message);
}

int main() {
    printf("This is a web application simulation in C.\\n");
    printf("C is not typically used for web applications.\\n\\n");
    
    printf("Test cases:\\n\\n");
    
    UserInput testCases[5] = {
        {"John Doe", "john@example.com", "Hello World"},
        {"Jane Smith", "jane@test.com", "Test message"},
        {"Bob Johnson", "bob@demo.com", "This is a sample"},
        {"Alice Brown", "alice@mail.com", "C web app test"},
        {"Charlie Wilson", "charlie@site.com", "Final test case"}
    };
    
    for (int i = 0; i < 5; i++) {
        printf("Test Case %d:\\n", i + 1);
        processInput(&testCases[i]);
        printf("\\n");
    }
    
    return 0;
}
