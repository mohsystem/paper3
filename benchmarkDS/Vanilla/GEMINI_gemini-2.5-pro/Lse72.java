import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse72 {

    /**
     * Creates an 'uploads' directory if it doesn't exist and saves a file with the given content into it.
     * The p.save(os.path.join('uploads' filename)) line from the prompt is conceptually implemented here.
     * @param filename The name of the file to save.
     * @param content The content to write into the file.
     * @return true if the file was saved successfully, false otherwise.
     */
    public static boolean uploadFile(String filename, String content) {
        String dirName = "uploads";
        try {
            // Create the 'uploads' directory if it does not exist
            Path dirPath = Paths.get(dirName);
            if (!Files.exists(dirPath)) {
                Files.createDirectories(dirPath);
            }

            // Create the full path for the file, analogous to os.path.join('uploads', filename)
            Path filePath = dirPath.resolve(filename);

            // Save the file, analogous to p.save(...)
            try (FileWriter writer = new FileWriter(filePath.toFile())) {
                writer.write(content);
            }
            
            System.out.println("Java: Successfully saved file to " + filePath.toAbsolutePath());
            return true;

        } catch (IOException e) {
            System.err.println("Java Error: Failed to save file. " + e.getMessage());
            return false;
        }
    }

    public static void main(String[] args) {
        // Run test cases for all languages if they are in the same execution context
        // This main method is specific to Java execution.
        
        System.out.println("--- Java Test Cases ---");
        uploadFile("java_test1.txt", "This is a test from Java.");
        uploadFile("config.json", "{ \"key\": \"value\" }");
        uploadFile("notes.txt", "Remember to buy milk.");
        uploadFile("log_01.txt", "Application started successfully.");
        uploadFile("report.csv", "ID,Name,Score\n1,Alice,95\n2,Bob,88");
        System.out.println("-----------------------\n");
    }
}