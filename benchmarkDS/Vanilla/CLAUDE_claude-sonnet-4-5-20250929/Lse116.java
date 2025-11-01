
public class Lse116 {
    public static void executeCommand(String command) {
        try {
            Process process = Runtime.getRuntime().exec(command);
            process.waitFor();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Execute ls command
        executeCommand("ls");
        
        // Test case 2: Execute ls with -l flag
        executeCommand("ls -l");
        
        // Test case 3: Execute ls with -a flag
        executeCommand("ls -a");
        
        // Test case 4: Execute ls with -lh flags
        executeCommand("ls -lh");
        
        // Test case 5: Execute ls for specific directory
        executeCommand("ls /tmp");
    }
}
