
import java.io.IOException;

public class Lse114 {
    public static void executeCommand(String directory) {
        try {
            String command = String.format("ls %s", directory);
            System.out.println("Executing: " + command);
            Runtime.getRuntime().exec(command);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    public static void main(String[] args) {
        // Test cases - INSECURE
        executeCommand("/tmp");
        executeCommand("/home");
        executeCommand("/var");
        executeCommand("/usr");
        executeCommand("/etc");
    }
}
