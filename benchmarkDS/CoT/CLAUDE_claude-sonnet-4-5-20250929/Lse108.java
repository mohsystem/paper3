
import java.io.*;
import java.util.regex.*;

public class Lse108 {
    private static final Pattern VALID_USERNAME = Pattern.compile("^[a-zA-Z0-9_-]{1,32}$");
    
    public static String getPasswdInfo(String username) {
        // Input validation to prevent command injection
        if (username == null || !VALID_USERNAME.matcher(username).matches()) {
            return "Error: Invalid username format";
        }
        
        StringBuilder output = new StringBuilder();
        Process process = null;
        BufferedReader reader = null;
        
        try {
            // Use ProcessBuilder for safer command execution
            ProcessBuilder pb = new ProcessBuilder("getent", "passwd", username);
            process = pb.start();
            
            reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line;
            
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            process.waitFor();
            
        } catch (IOException | InterruptedException e) {
            return "Error: " + e.getMessage();
        } finally {
            try {
                if (reader != null) reader.close();
                if (process != null) process.destroy();
            } catch (IOException e) {
                // Handle cleanup error
            }
        }
        
        return output.toString();
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1 - Valid user 'root':");
        System.out.println(getPasswdInfo("root"));
        
        System.out.println("\\nTest 2 - Valid user 'nobody':");
        System.out.println(getPasswdInfo("nobody"));
        
        System.out.println("\\nTest 3 - Invalid user with special chars '; rm -rf /':");
        System.out.println(getPasswdInfo("; rm -rf /"));
        
        System.out.println("\\nTest 4 - Empty username:");
        System.out.println(getPasswdInfo(""));
        
        System.out.println("\\nTest 5 - Null username:");
        System.out.println(getPasswdInfo(null));
    }
}
