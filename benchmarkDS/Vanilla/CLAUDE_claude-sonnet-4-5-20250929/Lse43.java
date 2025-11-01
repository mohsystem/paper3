
import java.io.*;
import java.nio.file.*;

public class Lse43 {
    public static String readFileFromSafeDirectory(String filename) {
        try {
            String safePath = "/safe/" + filename;
            return new String(Files.readAllBytes(Paths.get(safePath)));
        } catch (IOException e) {
            return "Error reading file: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testFiles = {
            "test1.txt",
            "test2.txt",
            "data.txt",
            "config.txt",
            "info.txt"
        };
        
        for (String file : testFiles) {
            System.out.println("Reading file: " + file);
            String content = readFileFromSafeDirectory(file);
            System.out.println("Content: " + content);
            System.out.println("---");
        }
    }
}
