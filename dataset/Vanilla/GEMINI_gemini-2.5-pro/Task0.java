import java.io.IOException;
import java.nio.file.*;
import java.util.Comparator;
import java.util.Queue;
import java.util.concurrent.*;
import java.util.stream.Stream;

public class Task0 {
    private static final String DIR_PATH = "java_test_files";
    private static final int NUM_FILES = 5; // Test case: 5 files
    private static final int NUM_THREADS = 3;

    // The worker thread logic
    static class FileProcessor implements Runnable {
        private final Queue<Path> fileQueue;
        private final int threadId;

        public FileProcessor(Queue<Path> fileQueue, int threadId) {
            this.fileQueue = fileQueue;
            this.threadId = threadId;
        }

        @Override
        public void run() {
            while (true) {
                Path filePath = fileQueue.poll(); // Atomically retrieve and remove
                if (filePath == null) {
                    break; // No more files left in the queue
                }
                processFile(filePath);
            }
        }

        private void processFile(Path filePath) {
            System.out.println("Thread " + threadId + " is processing file: " + filePath.getFileName());
            try {
                // Simulate processing time
                Thread.sleep(100);

                // Append a message to the file to show it was processed
                String content = "\nProcessed by Thread " + threadId;
                Files.write(filePath, content.getBytes(), StandardOpenOption.APPEND);

                System.out.println("Thread " + threadId + " finished processing file: " + filePath.getFileName());
            } catch (IOException | InterruptedException e) {
                System.err.println("Thread " + threadId + " encountered an error processing " + filePath.getFileName() + ": " + e.getMessage());
            }
        }
    }

    // Helper to set up the test environment
    private static void setupTestEnvironment() {
        System.out.println("Setting up test environment...");
        Path dir = Paths.get(DIR_PATH);
        try {
            if (Files.exists(dir)) {
                cleanupTestEnvironment();
            }
            Files.createDirectory(dir);
            for (int i = 1; i <= NUM_FILES; i++) {
                Path filePath = dir.resolve("test_file_" + i + ".txt");
                String initialContent = "This is test file " + i + ".";
                Files.write(filePath, initialContent.getBytes());
            }
            System.out.println(NUM_FILES + " test files created in " + DIR_PATH);
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
            System.exit(1);
        }
    }

    // Helper to clean up the test environment
    private static void cleanupTestEnvironment() {
        System.out.println("\nCleaning up test environment...");
        Path dir = Paths.get(DIR_PATH);
        if (Files.exists(dir)) {
            try (Stream<Path> walk = Files.walk(dir)) {
                walk.sorted(Comparator.reverseOrder())
                    .forEach(path -> {
                        try {
                            Files.delete(path);
                        } catch (IOException e) {
                            System.err.println("Failed to delete " + path + ": " + e.getMessage());
                        }
                    });
                System.out.println("Test directory and files deleted.");
            } catch (IOException e) {
                System.err.println("Failed to clean up test environment: " + e.getMessage());
            }
        }
    }

    public static void main(String[] args) {
        // 1. Setup: Create directory and 5 test files
        setupTestEnvironment();

        // 2. Create a thread-safe queue and populate it with files to process
        Queue<Path> fileQueue = new ConcurrentLinkedQueue<>();
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(Paths.get(DIR_PATH))) {
            for (Path entry : stream) {
                fileQueue.add(entry);
            }
        } catch (IOException e) {
            System.err.println("Error reading directory: " + e.getMessage());
            cleanupTestEnvironment();
            return;
        }

        System.out.println("\nStarting " + NUM_THREADS + " threads to process " + fileQueue.size() + " files.");
        
        // 3. Create a thread pool and submit tasks
        ExecutorService executor = Executors.newFixedThreadPool(NUM_THREADS);
        for (int i = 1; i <= NUM_THREADS; i++) {
            executor.submit(new FileProcessor(fileQueue, i));
        }

        // 4. Shut down the executor and wait for all threads to finish
        executor.shutdown();
        try {
            if (!executor.awaitTermination(60, TimeUnit.SECONDS)) {
                System.err.println("Threads did not finish in time.");
                executor.shutdownNow();
            }
        } catch (InterruptedException e) {
            executor.shutdownNow();
        }
        
        System.out.println("\nAll processing tasks are complete.");

        // 5. Cleanup: Remove the directory and its contents
        cleanupTestEnvironment();
    }
}