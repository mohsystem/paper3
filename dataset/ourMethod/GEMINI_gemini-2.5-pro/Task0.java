import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class Task0 {

    // Worker class that processes files from a shared queue
    private static class FileProcessor implements Runnable {
        private final ConcurrentLinkedQueue<Path> fileQueue;
        private final int threadId;

        public FileProcessor(int threadId, ConcurrentLinkedQueue<Path> fileQueue) {
            this.threadId = threadId;
            this.fileQueue = fileQueue;
        }

        @Override
        public void run() {
            while (true) {
                Path filePath = fileQueue.poll();
                if (filePath == null) {
                    // No more files in the queue
                    break;
                }
                processFile(filePath);
            }
        }

        private void processFile(Path filePath) {
            System.out.printf("Thread %d: Starting processing of %s%n", threadId, filePath.getFileName());
            try {
                // Simulate processing by reading the file and then sleeping
                // Using try-with-resources ensures the reader is closed
                // String content = new String(Files.readAllBytes(filePath), StandardCharsets.UTF_8);
                // For this example, we don't need the content, just the act of reading.
                Files.readAllBytes(filePath); 
                Thread.sleep(100); // Simulate work
                System.out.printf("Thread %d: Finished processing of %s%n", threadId, filePath.getFileName());
            } catch (IOException e) {
                System.err.printf("Thread %d: Error reading file %s: %s%n", threadId, filePath.getFileName(), e.getMessage());
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                System.err.printf("Thread %d: Interrupted while processing %s%n", threadId, filePath.getFileName());
            }
        }
    }

    public static void processFilesConcurrently(String directoryPath, int numThreads) throws IOException {
        Path dir = Paths.get(directoryPath).toAbsolutePath();
        if (!Files.isDirectory(dir)) {
            System.err.println("Error: Provided path is not a directory.");
            return;
        }

        ConcurrentLinkedQueue<Path> fileQueue = new ConcurrentLinkedQueue<>();
        try (Stream<Path> stream = Files.list(dir)) {
            List<Path> files = stream
                .filter(Files::isRegularFile)
                // Security: ensure we are not processing files outside the given directory
                .filter(path -> {
                    try {
                        return path.toAbsolutePath().normalize().startsWith(dir);
                    } catch (Exception e) {
                        return false;
                    }
                })
                .collect(Collectors.toList());
            fileQueue.addAll(files);
        }

        if (fileQueue.isEmpty()) {
            System.out.println("No files to process in the directory.");
            return;
        }

        ExecutorService executor = Executors.newFixedThreadPool(numThreads);
        for (int i = 0; i < numThreads; i++) {
            executor.submit(new FileProcessor(i + 1, fileQueue));
        }

        executor.shutdown();
        try {
            if (!executor.awaitTermination(60, TimeUnit.SECONDS)) {
                executor.shutdownNow();
            }
        } catch (InterruptedException e) {
            executor.shutdownNow();
            Thread.currentThread().interrupt();
        }
    }

    // Main method with test cases
    public static void main(String[] args) {
        // Create a temporary directory and files for testing
        Path tempDir = Paths.get("./temp_test_dir_java");
        try {
            // Setup
            if(Files.exists(tempDir)){
                // Clean up previous runs if they failed
                deleteDirectory(tempDir);
            }
            Files.createDirectory(tempDir);
            System.out.println("Created temporary directory: " + tempDir.toAbsolutePath());

            for (int i = 1; i <= 10; i++) {
                Path tempFile = tempDir.resolve("test_file_" + i + ".txt");
                try (PrintWriter writer = new PrintWriter(Files.newBufferedWriter(tempFile, StandardCharsets.UTF_8))) {
                    writer.println("This is test file " + i);
                }
            }

            // --- Test Cases ---
            System.out.println("\n--- Test Case 1: Processing 10 files with 4 threads ---");
            processFilesConcurrently(tempDir.toString(), 4);

            System.out.println("\n--- Test Case 2: Processing with 1 thread ---");
            processFilesConcurrently(tempDir.toString(), 1);

            System.out.println("\n--- Test Case 3: Processing with 8 threads ---");
            processFilesConcurrently(tempDir.toString(), 8);

            System.out.println("\n--- Test Case 4: Non-existent directory ---");
            processFilesConcurrently("./non_existent_dir", 4);
            
            System.out.println("\n--- Test Case 5: Empty directory ---");
            Path emptyDir = Paths.get("./empty_test_dir_java");
            Files.createDirectories(emptyDir);
            processFilesConcurrently(emptyDir.toString(), 4);
            Files.delete(emptyDir);

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            // Cleanup
            if (Files.exists(tempDir)) {
                try {
                    deleteDirectory(tempDir);
                    System.out.println("Cleaned up temporary directory.");
                } catch (IOException e) {
                    System.err.println("Error cleaning up temporary directory: " + e.getMessage());
                }
            }
        }
    }
    
    // Helper to delete directory recursively
    private static void deleteDirectory(Path path) throws IOException {
        try (Stream<Path> walk = Files.walk(path)) {
            walk.sorted(Comparator.reverseOrder())
                .map(Path::toFile)
                .forEach(File::delete);
        }
    }
}