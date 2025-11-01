
import java.io.*;

public class Lse108 {
    public static String getentPasswd(String username) {
        if (username == null || username.isEmpty()) {
            return "Error: Username cannot be null or empty";
        }
        
        try {
            // Execute the getent passwd command
            String command = "getent passwd " + username;
            Process process = Runtime.getRuntime().exec(command);
            
            // Read the output
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            
            StringBuilder output = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            // Wait for process to complete
            process.waitFor();
            reader.close();
            
            return output.toString().trim();
            
        } catch (Exception e) {
            return "Error executing command: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testUsers = {"root", "nobody", "daemon", "bin", "sys"};
        
        System.out.println("Testing getent passwd command:\\n");
        for (String user : testUsers) {
            System.out.println("User: " + user);
            System.out.println(getentPasswd(user));
            System.out.println("---");
        }
    }
}
