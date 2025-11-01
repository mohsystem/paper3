
import java.io.*;
import java.util.regex.Pattern;

public class Lse109 {
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_-]{1,32}$");
    
    public static String getUserInfo(String username) {
        // Validate username to prevent injection attacks
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            return "Invalid username format";
        }
        
        StringBuilder result = new StringBuilder();
        try (BufferedReader reader = new BufferedReader(new FileReader("/etc/passwd"))) {
            String line;
            while ((line = reader.readLine()) != null) {
                // Safely check if line starts with username followed by colon
                if (line.startsWith(username + ":")) {
                    result.append(line).append("\\n");
                }
            }
        } catch (FileNotFoundException e) {
            return "File not found: /etc/passwd";
        } catch (IOException e) {
            return "Error reading file: " + e.getMessage();
        }
        
        return result.length() > 0 ? result.toString().trim() : "User not found";
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {"root", "daemon", "nobody", "invalid@user", "testuser"};
        
        if (args.length > 0) {
            System.out.println(getUserInfo(args[0]));
        } else {
            System.out.println("Running test cases:");
            for (String username : testCases) {
                System.out.println("\\n[Test: " + username + "]");
                System.out.println(getUserInfo(username));
            }
        }
    }
}
