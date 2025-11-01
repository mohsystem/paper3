
import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Lse110 {
    public static int runIdCommand(String username) {
        try {
            Process process = Runtime.getRuntime().exec("id " + username);
            
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            String line;
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
            }
            
            BufferedReader errorReader = new BufferedReader(
                new InputStreamReader(process.getErrorStream())
            );
            while ((line = errorReader.readLine()) != null) {
                System.err.println(line);
            }
            
            int exitStatus = process.waitFor();
            return exitStatus;
        } catch (Exception e) {
            e.printStackTrace();
            return -1;
        }
    }
    
    public static void main(String[] args) {
        if (args.length > 0) {
            String username = args[0];
            int exitStatus = runIdCommand(username);
            System.out.println("Exit status: " + exitStatus);
        } else {
            // Test cases
            String[] testUsers = {"root", "nobody", "daemon", "nonexistentuser123", "bin"};
            
            for (String user : testUsers) {
                System.out.println("\\n=== Testing with user: " + user + " ===");
                int exitStatus = runIdCommand(user);
                System.out.println("Exit status: " + exitStatus);
            }
        }
    }
}
