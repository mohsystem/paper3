
import java.io.*;
import java.net.*;
import java.util.Scanner;

public class Lse126 {
    
    public static String htmlEscape(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;")
                   .replace("/", "&#x2F;");
    }
    
    public static void http_hello_page(InputStream requestBody, OutputStream response) {
        try {
            // Read username from request body with size limit
            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            byte[] data = new byte[1024];
            int bytesRead;
            int totalBytes = 0;
            int maxSize = 8192; // 8KB limit
            
            while ((bytesRead = requestBody.read(data)) != -1 && totalBytes < maxSize) {
                buffer.write(data, 0, bytesRead);
                totalBytes += bytesRead;
            }
            
            String username = buffer.toString("UTF-8").trim();
            
            // Validate and sanitize username
            if (username.length() > 100) {
                username = username.substring(0, 100);
            }
            username = htmlEscape(username);
            
            // Construct HTML response
            StringBuilder outputbuf = new StringBuilder();
            outputbuf.append("HTTP/1.1 200 OK\\r\\n");
            outputbuf.append("Content-Type: text/html; charset=UTF-8\\r\\n");
            outputbuf.append("X-Content-Type-Options: nosniff\\r\\n");
            outputbuf.append("X-Frame-Options: DENY\\r\\n");
            outputbuf.append("\\r\\n");
            outputbuf.append("<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">");
            outputbuf.append("<title>Hello Page</title></head><body>");
            outputbuf.append("<h1>Hello, ").append(username).append("!</h1>");
            outputbuf.append("</body></html>");
            
            // Write response
            response.write(outputbuf.toString().getBytes("UTF-8"));
            response.flush();
            
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                response.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testUsernames = {"Alice", "Bob<script>alert('xss')</script>", 
                                 "Charlie&Sons", "Diana\\"O'Neil\\"", ""};
        
        for (String testUser : testUsernames) {
            System.out.println("Testing with username: " + testUser);
            ByteArrayInputStream input = new ByteArrayInputStream(testUser.getBytes());
            ByteArrayOutputStream output = new ByteArrayOutputStream();
            http_hello_page(input, output);
            System.out.println(output.toString());
            System.out.println("---");
        }
    }
}
