
import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Lse109 {
    public static String executeCommand(String username) {
        StringBuilder output = new StringBuilder();
        try {
            String command = "cat /etc/passwd | grep " + username;
            Process process = Runtime.getRuntime().exec(new String[]{"sh", "-c", command});
            
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            process.waitFor();
            reader.close();
        } catch (Exception e) {
            return "Error executing command: " + e.getMessage();
        }
        return output.toString();
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testUsers = {"root", "daemon", "bin", "sys", "nobody"};
        
        if (args.length > 0) {
            String username = args[0];
            System.out.println("User information for: " + username);
            System.out.println(executeCommand(username));
        } else {
            System.out.println("Running test cases:");
            for (String user : testUsers) {
                System.out.println("=== User: " + user + " ===");
                System.out.println(executeCommand(user));
            }
        }
    }
}
