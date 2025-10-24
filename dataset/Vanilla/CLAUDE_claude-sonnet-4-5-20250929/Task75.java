
public class Task75 {
    public static void terminateProcess(int pid) {
        try {
            String os = System.getProperty("os.name").toLowerCase();
            Process process;
            
            if (os.contains("win")) {
                process = Runtime.getRuntime().exec("taskkill /F /PID " + pid);
            } else {
                process = Runtime.getRuntime().exec("kill -9 " + pid);
            }
            
            int exitCode = process.waitFor();
            if (exitCode == 0) {
                System.out.println("Process " + pid + " terminated successfully.");
            } else {
                System.out.println("Failed to terminate process " + pid + ".");
            }
        } catch (Exception e) {
            System.out.println("Error terminating process: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        if (args.length > 0) {
            try {
                int pid = Integer.parseInt(args[0]);
                terminateProcess(pid);
            } catch (NumberFormatException e) {
                System.out.println("Invalid PID format. Please provide a valid integer.");
            }
        } else {
            System.out.println("Running test cases...");
            System.out.println("\\nNote: These are demonstration test cases.");
            System.out.println("Please provide actual PIDs to terminate real processes.");
            System.out.println("\\nTest Case 1: PID 99999 (likely non-existent)");
            terminateProcess(99999);
            
            System.out.println("\\nTest Case 2: PID 88888 (likely non-existent)");
            terminateProcess(88888);
            
            System.out.println("\\nTest Case 3: PID 77777 (likely non-existent)");
            terminateProcess(77777);
            
            System.out.println("\\nTest Case 4: PID 66666 (likely non-existent)");
            terminateProcess(66666);
            
            System.out.println("\\nTest Case 5: PID 55555 (likely non-existent)");
            terminateProcess(55555);
        }
    }
}
