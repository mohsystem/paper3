
import java.io.*;
import java.util.Scanner;

public class Lse127 {
    public static void httpHelloPage(InputStream request, OutputStream response) {
        try {
            Scanner scanner = new Scanner(request);
            String username = "";
            
            // Read from request to find username parameter
            while (scanner.hasNextLine()) {
                String line = scanner.nextLine();
                if (line.contains("username=")) {
                    int startIndex = line.indexOf("username=") + 9;
                    int endIndex = line.indexOf("&", startIndex);
                    if (endIndex == -1) {
                        endIndex = line.indexOf(" ", startIndex);
                    }
                    if (endIndex == -1) {
                        endIndex = line.length();
                    }
                    username = line.substring(startIndex, endIndex);
                    break;
                }
            }
            
            // Construct response
            String responseMessage = "Hello " + username;
            PrintWriter writer = new PrintWriter(response, true);
            writer.println("HTTP/1.1 200 OK");
            writer.println("Content-Type: text/html");
            writer.println();
            writer.println(responseMessage);
            writer.flush();
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        String request1 = "GET /hello?username=John HTTP/1.1\\n";
        ByteArrayInputStream input1 = new ByteArrayInputStream(request1.getBytes());
        ByteArrayOutputStream output1 = new ByteArrayOutputStream();
        httpHelloPage(input1, output1);
        System.out.println("Test 1:\\n" + output1.toString() + "\\n");
        
        // Test case 2
        String request2 = "POST /hello\\nusername=Alice";
        ByteArrayInputStream input2 = new ByteArrayInputStream(request2.getBytes());
        ByteArrayOutputStream output2 = new ByteArrayOutputStream();
        httpHelloPage(input2, output2);
        System.out.println("Test 2:\\n" + output2.toString() + "\\n");
        
        // Test case 3
        String request3 = "GET /hello?username=Bob&age=25 HTTP/1.1\\n";
        ByteArrayInputStream input3 = new ByteArrayInputStream(request3.getBytes());
        ByteArrayOutputStream output3 = new ByteArrayOutputStream();
        httpHelloPage(input3, output3);
        System.out.println("Test 3:\\n" + output3.toString() + "\\n");
        
        // Test case 4
        String request4 = "GET /hello?username=Charlie HTTP/1.1\\n";
        ByteArrayInputStream input4 = new ByteArrayInputStream(request4.getBytes());
        ByteArrayOutputStream output4 = new ByteArrayOutputStream();
        httpHelloPage(input4, output4);
        System.out.println("Test 4:\\n" + output4.toString() + "\\n");
        
        // Test case 5
        String request5 = "POST /hello\\nusername=Eve&password=secret";
        ByteArrayInputStream input5 = new ByteArrayInputStream(request5.getBytes());
        ByteArrayOutputStream output5 = new ByteArrayOutputStream();
        httpHelloPage(input5, output5);
        System.out.println("Test 5:\\n" + output5.toString() + "\\n");
    }
}
