import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.stream.Stream;

public class Task0 {

    /**
     * Represents a worker thread that processes files from a shared queue.
     */
    static class FileProcessor implements Runnable {
        private final Queue<Path> fileQueue;

        public FileProcessor(Queue<Path> fileQueue) {
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

        /**
         * Simulates processing a single file. This method ensures that each file is
         * handled by exactly one thread at a time because it's only called for a
         * file path that has been exclusively removed from the shared queue.
         *
         * @param filePath The path to the file to be processed.
         */
        private void processFile(Path filePath) {
            try {
                System.out.println(Thread.currentThread().getName() + " started processing: " + filePath.getFileName());
                // Simulate some work, e.g., reading content, data processing
                Thread.sleep((long) (Math.random() * 100 + 50));
                System.out.println(Thread.currentThread().getName() + " finished processing: " + filePath.getFileName());
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                System.err.println("File processing was interrupted for: " + filePath.getFileName());
            }
        }
    }

    /**
     * Sets up the environment, runs the simulation, and cleans up.
     *
     * @param directoryPath The path to the directory to be created for the test.
     * @param numFiles      The number of files to create for the test.
     * @param numThreads    The number of threads to use for processing.
     */
    public static void runTest(String directoryPath, int numFiles, int numThreads) throws IOException, InterruptedException {
        System.out.println("\n--- Running Test ---");
        System.out.printf("Directory: %s, Files: %d, Threads: %d%n", directoryPath, numFiles, numThreads);

        Path dir = Paths.get(directoryPath);
        
        try {
            // 1. Setup: Create directory and dummy files
            Files.createDirectories(dir);
            for (int i = 1; i <= numFiles; i++) {
                Files.createFile(dir.resolve("file_" + i + ".txt"));
            }

            // 2. Create a shared, thread-safe queue and populate it with files to process
            Queue<Path> fileQueue = new ConcurrentLinkedQueue<>();
            try (Stream<Path> paths = Files.list(dir)) {
                paths.filter(Files::isRegularFile).forEach(fileQueue::add);
            }

            // 3. Setup and run thread pool
            ExecutorService executor = Executors.newFixedThreadPool(numThreads);
            for (int i = 0; i < numThreads; i++) {
                executor.submit(new FileProcessor(fileQueue));
            }

            // 4. Shutdown executor and wait for completion
            executor.shutdown();
            if (!executor.awaitTermination(60, TimeUnit.SECONDS)) {
                System.err.println("Threads did not finish in 60 seconds!");
                executor.shutdownNow();
            }
        } finally {
            // 5. Cleanup: Delete the directory and its contents
             if (Files.exists(dir)) {
                Files.walkFileTree(dir, new SimpleFileVisitor<Path>() {
                    @Override
                    public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException {
                        Files.delete(file);
                        return FileVisitResult.CONTINUE;
                    }
                    @Override
                    public FileVisitResult postVisitDirectory(Path dir, IOException exc) throws IOException {
                        Files.delete(dir);
                        return FileVisitResult.CONTINUE;
                    }
                });
                System.out.println("Cleaned up directory: " + directoryPath);
            }
        }
        System.out.println("--- Test Finished ---");
    }

    public static void main(String[] args) {
        try {
            // Test Case 1: More files than threads
            runTest("java_test_dir_1", 5, 2);
            // Test Case 2: Equal files and threads
            runTest("java_test_dir_2", 4, 4);
            // Test Case 3: More threads than files
            runTest("java_test_dir_3", 3, 5);
            // Test Case 4: Single thread
            runTest("java_test_dir_4", 5, 1);
            // Test Case 5: Large number of files
            runTest("java_test_dir_5", 20, 4);
        } catch (IOException | InterruptedException e) {
            System.err.println("An error occurred during the test execution: " + e.getMessage());
            e.printStackTrace();
        }
    }
}