
public class Lse116 {
    public static void executeCommand() {
        // WARNING: Using Runtime.exec or ProcessBuilder to execute system commands
        // can be dangerous if any user input is involved. This example uses a
        // hardcoded command only.
        
        // Secure approach: use ProcessBuilder with explicit command and arguments
        // Never concatenate user input into commands
        try {
            ProcessBuilder pb = new ProcessBuilder("ls");
            Process process = pb.start();
            
            // Read output safely
            java.io.BufferedReader reader = new java.io.BufferedReader(
                new java.io.InputStreamReader(process.getInputStream()));
            String line;
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
            }
            
            int exitCode = process.waitFor();
            System.out.println("Command exited with code: " + exitCode);
            
        } catch (Exception e) {
            System.err.println("Error executing command");
        }
    }
    
    public static void main(String[] args) {
        // Test cases - all execute the same hardcoded safe command
        System.out.println("Test 1:");
        executeCommand();
        
        System.out.println("\\nTest 2:");
        executeCommand();
        
        System.out.println("\\nTest 3:");
        executeCommand();
        
        System.out.println("\\nTest 4:");
        executeCommand();
        
        System.out.println("\\nTest 5:");
        executeCommand();
    }
}
