
import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Lse115 {
    public static String executeCommand(String command) {
        StringBuilder output = new StringBuilder();
        try {
            Process process = Runtime.getRuntime().exec(command);
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
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

    public static String formatCommand(String format, String... args) {
        return String.format(format, (Object[]) args);
    }

    public static void main(String[] args) {
        // Test case 1: List current directory
        String cmd1 = formatCommand("ls %s", ".");
        System.out.println("Test 1: " + cmd1);
        System.out.println(executeCommand(cmd1));

        // Test case 2: List with -l flag
        String cmd2 = formatCommand("ls %s", "-l");
        System.out.println("Test 2: " + cmd2);
        System.out.println(executeCommand(cmd2));

        // Test case 3: List with -a flag
        String cmd3 = formatCommand("ls %s", "-a");
        System.out.println("Test 3: " + cmd3);
        System.out.println(executeCommand(cmd3));

        // Test case 4: List with -la flags
        String cmd4 = formatCommand("ls %s %s", "-l", "-a");
        System.out.println("Test 4: " + cmd4);
        System.out.println(executeCommand(cmd4));

        // Test case 5: List specific directory
        String cmd5 = formatCommand("ls %s", "/tmp");
        System.out.println("Test 5: " + cmd5);
        System.out.println(executeCommand(cmd5));
    }
}
