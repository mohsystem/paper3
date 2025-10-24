
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.Scanner;

public class Task39 {
    // INSECURE: Vulnerable to command injection
    public static String displayFileContents(String filename) {
        StringBuilder output = new StringBuilder();
        try {
            String command;
            String os = System.getProperty("os.name").toLowerCase();
            
            if (os.contains("win")) {
                command = "type " + filename;
            } else {
                command = "cat " + filename;
            }
            
            Process process = Runtime.getRuntime().exec(command);
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            BufferedReader errorReader = new BufferedReader(
                new InputStreamReader(process.getErrorStream())
            );
            while ((line = errorReader.readLine()) != null) {
                output.append("ERROR: ").append(line).append("\\n");
            }
            
            process.waitFor();
            reader.close();
            errorReader.close();
            
        } catch (Exception e) {
            output.append("Error: ").append(e.getMessage());
        }
        return output.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("WARNING: This code is vulnerable to command injection!");
        System.out.println("Test cases:\\n");
        
        String[] testCases = {
            "test.txt",
            "document.log",
            "data.csv",
            "config.ini",
            "readme.md"
        };
        
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ": " + testCases[i]);
            System.out.println(displayFileContents(testCases[i]));
            System.out.println("---");
        }
    }
}
