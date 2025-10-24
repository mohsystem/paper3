
import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Task75 {
    public static String terminateProcess(String pid) {
        try {
            // Validate PID is numeric
            int processId = Integer.parseInt(pid);
            if (processId <= 0) {
                return "Error: Invalid PID. Must be a positive number.";
            }
            
            String os = System.getProperty("os.name").toLowerCase();
            Process process;
            
            if (os.contains("win")) {
                // Windows command
                process = Runtime.getRuntime().exec("taskkill /F /PID " + processId);
            } else {
                // Unix/Linux/Mac command
                process = Runtime.getRuntime().exec("kill -9 " + processId);
            }
            
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            BufferedReader errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()));
            
            StringBuilder output = new StringBuilder();
            String line;
            
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            while ((line = errorReader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            int exitCode = process.waitFor();
            
            if (exitCode == 0) {
                return "Process " + processId + " terminated successfully.";
            } else {
                return "Error terminating process " + processId + ": " + output.toString();
            }
            
        } catch (NumberFormatException e) {
            return "Error: Invalid PID format. Must be a number.";
        } catch (SecurityException e) {
            return "Error: Permission denied. Run as administrator/root.";
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        if (args.length > 0) {
            System.out.println(terminateProcess(args[0]));
        } else {
            // Test cases (Note: These are example PIDs that likely don't exist)
            System.out.println("Test Case 1 (Invalid PID - negative):");
            System.out.println(terminateProcess("-1"));
            
            System.out.println("\\nTest Case 2 (Invalid PID - zero):");
            System.out.println(terminateProcess("0"));
            
            System.out.println("\\nTest Case 3 (Invalid format):");
            System.out.println(terminateProcess("abc"));
            
            System.out.println("\\nTest Case 4 (Non-existent PID):");
            System.out.println(terminateProcess("999999"));
            
            System.out.println("\\nTest Case 5 (Empty string):");
            System.out.println(terminateProcess(""));
        }
    }
}
