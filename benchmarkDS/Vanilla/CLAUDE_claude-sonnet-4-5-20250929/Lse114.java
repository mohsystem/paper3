
public class Lse114 {
    public static void executeCommand(String command) {
        try {
            Process process = Runtime.getRuntime().exec(command);
            java.io.BufferedReader reader = new java.io.BufferedReader(
                new java.io.InputStreamReader(process.getInputStream())
            );
            String line;
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
            }
            process.waitFor();
            reader.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    public static void executeLsCommand(String directory) {
        String command = String.format("ls %s", directory);
        System.out.println("Executing: " + command);
        executeCommand(command);
    }
    
    public static void main(String[] args) {
        // Test case 1: List current directory
        System.out.println("Test Case 1: Current directory");
        executeLsCommand(".");
        
        // Test case 2: List home directory
        System.out.println("\\nTest Case 2: Home directory");
        executeLsCommand("~");
        
        // Test case 3: List with wildcard
        System.out.println("\\nTest Case 3: List with wildcard");
        executeLsCommand("*.java");
        
        // Test case 4: List parent directory
        System.out.println("\\nTest Case 4: Parent directory");
        executeLsCommand("..");
        
        // Test case 5: List /tmp directory
        System.out.println("\\nTest Case 5: /tmp directory");
        executeLsCommand("/tmp");
    }
}
