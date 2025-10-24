
import java.util.*;
import java.io.*;

public class Task120 {
    // HTML escape function to prevent XSS attacks
    public static String escapeHtml(String input) {
        if (input == null) {
            return "";
        }
        StringBuilder escaped = new StringBuilder();
        for (char c : input.toCharArray()) {
            switch (c) {
                case '<':
                    escaped.append("&lt;");
                    break;
                case '>':
                    escaped.append("&gt;");
                    break;
                case '&':
                    escaped.append("&amp;");
                    break;
                case '"':
                    escaped.append("&quot;");
                    break;
                case '\\'':
                    escaped.append("&#x27;");
                    break;
                case '/':
                    escaped.append("&#x2F;");
                    break;
                default:
                    escaped.append(c);
            }
        }
        return escaped.toString();
    }
    
    public static String displayUserInput(String userInput) {
        String sanitizedInput = escapeHtml(userInput);
        StringBuilder html = new StringBuilder();
        html.append("<!DOCTYPE html>\\n");
        html.append("<html>\\n");
        html.append("<head>\\n");
        html.append("    <meta charset=\\"UTF-8\\">\\n");
        html.append("    <title>User Input Display</title>\\n");
        html.append("</head>\\n");
        html.append("<body>\\n");
        html.append("    <h1>User Input Display</h1>\\n");
        html.append("    <p>You entered: ").append(sanitizedInput).append("</p>\\n");
        html.append("</body>\\n");
        html.append("</html>");
        return html.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Normal Text ===");
        String test1 = "Hello, World!";
        System.out.println(displayUserInput(test1));
        System.out.println();
        
        System.out.println("=== Test Case 2: XSS Attack Attempt (Script Tag) ===");
        String test2 = "<script>alert('XSS')</script>";
        System.out.println(displayUserInput(test2));
        System.out.println();
        
        System.out.println("=== Test Case 3: Special Characters ===");
        String test3 = "Price: $100 & Free Shipping!";
        System.out.println(displayUserInput(test3));
        System.out.println();
        
        System.out.println("=== Test Case 4: HTML Injection Attempt ===");
        String test4 = "<img src=x onerror=alert('XSS')>";
        System.out.println(displayUserInput(test4));
        System.out.println();
        
        System.out.println("=== Test Case 5: Mixed Content ===");
        String test5 = "My email: user@example.com & phone: 555-1234";
        System.out.println(displayUserInput(test5));
    }
}
