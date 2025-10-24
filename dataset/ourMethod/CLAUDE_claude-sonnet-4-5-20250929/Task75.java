
import java.util.regex.Pattern;

public class Task75 {
    private static final Pattern PID_PATTERN = Pattern.compile("^[1-9][0-9]{0,9}$");
    private static final int MAX_PID = Integer.MAX_VALUE;
    
    public static String terminateProcess(String pidString) {
        // Validate input is not null or empty
        if (pidString == null || pidString.trim().isEmpty()) {
            return "Error: PID cannot be null or empty";
        }
        
        // Sanitize input - remove whitespace
        pidString = pidString.trim();
        
        // Validate PID format (positive integer only)
        if (!PID_PATTERN.matcher(pidString).matches()) {
            return "Error: Invalid PID format. Must be a positive integer";
        }
        
        // Parse and validate range
        long pidLong;
        try {
            pidLong = Long.parseLong(pidString);
            if (pidLong <= 0 || pidLong > MAX_PID) {
                return "Error: PID out of valid range (1-" + MAX_PID + ")";
            }
        } catch (NumberFormatException e) {
            return "Error: Invalid PID number";
        }
        
        int pid = (int) pidLong;
        
        // Attempt to terminate process
        try {
            ProcessBuilder pb = new ProcessBuilder();
            String os = System.getProperty("os.name").toLowerCase();
            
            if (os.contains("win")) {
                pb.command("taskkill", "/F", "/PID", String.valueOf(pid));
            } else {
                pb.command("kill", "-9", String.valueOf(pid));
            }
            
            Process process = pb.start();
            int exitCode = process.waitFor();
            
            if (exitCode == 0) {
                return "Success: Process " + pid + " terminated";
            } else {
                return "Error: Failed to terminate process " + pid;
            }
        } catch (Exception e) {
            return "Error: Exception occurred - " + e.getClass().getSimpleName();
        }
    }
    
    public static void main(String[] args) {
        if (args.length != 1) {
            System.err.println("Usage: java Task75 <PID>");
            System.exit(1);
        }
        
        String result = terminateProcess(args[0]);
        System.out.println(result);
        
        // Test cases
        System.out.println("\\nTest Cases:");
        System.out.println("Test 1 (valid format): " + terminateProcess("12345"));
        System.out.println("Test 2 (null): " + terminateProcess(null));
        System.out.println("Test 3 (empty): " + terminateProcess(""));
        System.out.println("Test 4 (negative): " + terminateProcess("-100"));
        System.out.println("Test 5 (invalid chars): " + terminateProcess("12a45"));
    }
}
