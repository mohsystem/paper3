import java.io.File;
import java.io.IOException;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.lang.reflect.Method;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

// Custom annotation to simulate a URL decorator
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.METHOD)
@interface Route {
    String value();
}

class Lse47 {

    private static final String IMAGE_DIR = "images";

    // The method that handles the request, decorated with our custom annotation
    @Route("/images/(?<filename>[a-zA-Z0-9_.-]+)")
    public String serveImage(String filename) {
        // --- Security Check: Path Traversal ---
        // Basic validation to prevent directory traversal characters.
        if (filename == null || filename.contains("..") || filename.contains("/") || filename.contains("\\")) {
            return "HTTP 400: Invalid filename.";
        }

        try {
            Path imageDir = Paths.get(IMAGE_DIR).toRealPath();
            Path requestedFile = imageDir.resolve(filename).toRealPath();

            // Final check: Ensure the resolved file path is still within the designated image directory.
            if (!requestedFile.startsWith(imageDir)) {
                return "HTTP 403: Forbidden - Access outside of image directory is not allowed.";
            }

            if (Files.exists(requestedFile) && Files.isRegularFile(requestedFile)) {
                 // Simulate sending file content to browser
                return "HTTP 200: Serving content of " + filename + ": " + new String(Files.readAllBytes(requestedFile));
            } else {
                return "HTTP 404: File not found.";
            }
        } catch (IOException e) {
            // This can happen if the file doesn't exist or if toRealPath() fails.
            return "HTTP 404: File not found or an I/O error occurred.";
        }
    }

    // A simple router to dispatch requests based on annotations
    static class SimpleRouter {
        private final Map<Pattern, Method> routeMap = new HashMap<>();
        private final Object controller;

        public SimpleRouter(Object controller) {
            this.controller = controller;
            for (Method method : controller.getClass().getDeclaredMethods()) {
                if (method.isAnnotationPresent(Route.class)) {
                    Route route = method.getAnnotation(Route.class);
                    routeMap.put(Pattern.compile("^" + route.value() + "$"), method);
                }
            }
        }

        public String dispatch(String url) {
            for (Map.Entry<Pattern, Method> entry : routeMap.entrySet()) {
                Matcher matcher = entry.getKey().matcher(url);
                if (matcher.matches()) {
                    try {
                        String filename = matcher.group("filename");
                        Method method = entry.getValue();
                        return (String) method.invoke(controller, filename);
                    } catch (Exception e) {
                        return "HTTP 500: Internal Server Error - " + e.getMessage();
                    }
                }
            }
            return "HTTP 404: Route not found.";
        }
    }

    // Main method with test cases
    public static void main(String[] args) {
        // 1. Setup: Create a dummy images directory and files
        try {
            Path dir = Paths.get(IMAGE_DIR);
            Files.createDirectories(dir);
            Files.write(dir.resolve("cat.jpg"), "This is a cat image".getBytes());
            Files.write(dir.resolve("dog_1.png"), "This is a dog image".getBytes());
        } catch (IOException e) {
            System.err.println("Failed to set up test files: " + e.getMessage());
            return;
        }

        Lse47 app = new Lse47();
        SimpleRouter router = new SimpleRouter(app);

        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Valid file request
        System.out.println("Test 1 (Valid): /images/cat.jpg");
        System.out.println("Response: " + router.dispatch("/images/cat.jpg"));
        System.out.println();

        // Test Case 2: File does not exist
        System.out.println("Test 2 (Not Found): /images/horse.gif");
        System.out.println("Response: " + router.dispatch("/images/horse.gif"));
        System.out.println();
        
        // Test Case 3: Path traversal attempt
        System.out.println("Test 3 (Path Traversal): /images/../secret.txt");
        System.out.println("Response: " + router.dispatch("/images/../secret.txt")); // Will fail regex
        System.out.println();
        
        // Test Case 4: Another path traversal attempt (different format)
        System.out.println("Test 4 (Path Traversal 2): /images/..%2fsecret.txt");
        System.out.println("Response: " + router.dispatch("/images/..%2fsecret.txt")); // Will fail regex
        System.out.println();

        // Test Case 5: Invalid route
        System.out.println("Test 5 (Invalid Route): /videos/movie.mp4");
        System.out.println("Response: " + router.dispatch("/videos/movie.mp4"));
        System.out.println();

        // Cleanup
        try {
            Files.deleteIfExists(Paths.get(IMAGE_DIR, "cat.jpg"));
            Files.deleteIfExists(Paths.get(IMAGE_DIR, "dog_1.png"));
            Files.deleteIfExists(Paths.get(IMAGE_DIR));
        } catch (IOException e) {
            System.err.println("Failed to clean up test files.");
        }
    }
}