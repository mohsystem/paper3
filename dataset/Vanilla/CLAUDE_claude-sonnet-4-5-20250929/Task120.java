
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.util.Scanner;

public class Task120 {
    // Method to sanitize HTML to prevent XSS attacks
    public static String sanitizeHTML(String input) {
        if (input == null) {
            return "";
        }
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;")
                   .replace("/", "&#x2F;");
    }
    
    // Method to generate HTML page with user input
    public static String generateWebPage(String userInput) {
        String sanitizedInput = sanitizeHTML(userInput);
        
        StringBuilder html = new StringBuilder();
        html.append("<!DOCTYPE html>\\n");
        html.append("<html>\\n");
        html.append("<head>\\n");
        html.append("    <title>User Input Display</title>\\n");
        html.append("    <style>\\n");
        html.append("        body { font-family: Arial, sans-serif; margin: 40px; }\\n");
        html.append("        .container { border: 2px solid #333; padding: 20px; border-radius: 5px; }\\n");
        html.append("        h1 { color: #333; }\\n");
        html.append("    </style>\\n");
        html.append("</head>\\n");
        html.append("<body>\\n");
        html.append("    <div class=\\"container\\">\\n");
        html.append("        <h1>User Input Display</h1>\\n");
        html.append("        <p><strong>You entered:</strong></p>\\n");
        html.append("        <p>" + sanitizedInput + "</p>\\n");
        html.append("    </div>\\n");
        html.append("</body>\\n");
        html.append("</html>");
        
        return html.toString();
    }
    
    // Main method with test cases
    public static void main(String[] args) {
        System.out.println("Test Case 1: Simple text");
        System.out.println(generateWebPage("Hello, World!"));
        System.out.println("\\n" + "=".repeat(80) + "\\n");
        
        System.out.println("Test Case 2: Text with HTML tags (XSS prevention)");
        System.out.println(generateWebPage("<script>alert('XSS')</script>"));
        System.out.println("\\n" + "=".repeat(80) + "\\n");
        
        System.out.println("Test Case 3: Text with special characters");
        System.out.println(generateWebPage("Price: $100 & up, 50% off!"));
        System.out.println("\\n" + "=".repeat(80) + "\\n");
        
        System.out.println("Test Case 4: Multi-line text");
        System.out.println(generateWebPage("Line 1\\nLine 2\\nLine 3"));
        System.out.println("\\n" + "=".repeat(80) + "\\n");
        
        System.out.println("Test Case 5: Empty input");
        System.out.println(generateWebPage(""));
        System.out.println("\\n" + "=".repeat(80) + "\\n");
    }
}
