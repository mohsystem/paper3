import io.javalin.Javalin;
import io.javalin.http.Context;
import io.javalin.http.HttpStatus;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.nio.file.Path;
import java.nio.file.Paths;

// Note: This code requires the Javalin framework.
// Add the following dependencies to your pom.xml (Maven) or build.gradle (Gradle):
//
// <!-- Maven pom.xml -->
// <dependencies>
//     <dependency>
//         <groupId>io.javalin</groupId>
//         <artifactId>javalin</artifactId>
//         <version>5.6.3</version>
//     </dependency>
//     <dependency>
//         <groupId>org.slf4j</groupId>
//         <artifactId>slf4j-simple</artifactId>
//         <version>2.0.7</version>
//     </dependency>
// </dependencies>
public class Lse46 {

    private static final String IMAGE_DIR = "images";

    /**
     * The img function that handles requests to /img.
     * It's associated with the URL in the main method.
     * @param ctx The context object for the HTTP request and response.
     */
    public static void img(Context ctx) {
        String fileName = ctx.queryParam("name");

        if (fileName == null || fileName.trim().isEmpty()) {
            ctx.status(HttpStatus.BAD_REQUEST).result("Missing 'name' parameter.");
            return;
        }

        // Security: Prevent path traversal by ensuring the resolved file path
        // is within the designated 'images' directory.
        try {
            Path imageDirPath = Paths.get(IMAGE_DIR).toAbsolutePath();
            Path requestedFilePath = imageDirPath.resolve(fileName).normalize();

            // This is the critical security check. If the normalized path does not
            // start with the base directory path, it's a traversal attempt.
            if (!requestedFilePath.startsWith(imageDirPath)) {
                ctx.status(HttpStatus.NOT_FOUND).result("File not found.");
                return;
            }

            File file = requestedFilePath.toFile();

            if (file.exists() && file.isFile()) {
                // Safely stream the file back to the browser.
                ctx.result(new FileInputStream(file));
            } else {
                ctx.status(HttpStatus.NOT_FOUND).result("File not found.");
            }
        } catch (FileNotFoundException e) {
            ctx.status(HttpStatus.NOT_FOUND).result("File not found.");
        } catch (Exception e) {
            // Do not expose internal error details to the client.
            System.err.println("Internal server error: " + e.getMessage());
            ctx.status(HttpStatus.INTERNAL_SERVER_ERROR).result("An internal error occurred.");
        }
    }

    /**
     * Sets up a test directory and a sample file.
     */
    private static void setupTestEnvironment() {
        System.out.println("Setting up Java test environment...");
        File dir = new File(IMAGE_DIR);
        if (!dir.exists()) {
            dir.mkdirs();
            System.out.println("Created directory: " + dir.getAbsolutePath());
        }

        File testFile = new File(dir, "test.txt");
        try (PrintWriter writer = new PrintWriter(testFile)) {
            writer.println("This is a test file from Java.");
        } catch (FileNotFoundException e) {
            System.err.println("Failed to create test file: " + e.getMessage());
        }
        System.out.println("Created test file: " + testFile.getAbsolutePath());
    }

    public static void main(String[] args) {
        setupTestEnvironment();
        int port = 8080;

        // Create and start the Javalin server.
        Javalin app = Javalin.create().start(port);

        // This line is the equivalent of a decorator in other languages.
        // It associates the URL "/img" with the Lse46::img function.
        app.get("/img", Lse46::img);

        System.out.println("\n--- Java Javalin Server ---");
        System.out.println("Server running on http://127.0.0.1:" + port);
        System.out.println("Press Ctrl+C to stop.");
        System.out.println("\n--- 5 Test Cases ---");
        System.out.println("1. Valid file:         http://127.0.0.1:" + port + "/img?name=test.txt");
        System.out.println("2. File not found:     http://127.0.0.1:" + port + "/img?name=nonexistent.txt");
        System.out.println("3. Path traversal 1:   http://127.0.0.1:" + port + "/img?name=../pom.xml (should be 404)");
        System.out.println("4. Path traversal 2:   http://127.0.0.1:" + port + "/img?name=..%2ftest.txt (should be 404)");
        System.out.println("5. Missing parameter:  http://127.0.0.1:" + port + "/img (should be 400)");
    }
}