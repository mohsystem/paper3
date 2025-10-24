
import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.locks.*;

class Task0 {
    private final ConcurrentHashMap<String, ReentrantLock> fileLocks;
    private final ExecutorService executorService;
    
    public Task0(int threadPoolSize) {
        this.fileLocks = new ConcurrentHashMap<>();
        this.executorService = Executors.newFixedThreadPool(threadPoolSize);
    }
    
    public void processFile(String filePath, FileProcessor processor) {
        executorService.submit(() -> {
            ReentrantLock lock = fileLocks.computeIfAbsent(filePath, k -> new ReentrantLock());
            
            try {
                lock.lock();
                System.out.println(Thread.currentThread().getName() + " acquired lock for: " + filePath);
                processor.process(filePath);
                System.out.println(Thread.currentThread().getName() + " finished processing: " + filePath);
            } catch (Exception e) {
                System.err.println("Error processing file " + filePath + ": " + e.getMessage());
            } finally {
                lock.unlock();
                System.out.println(Thread.currentThread().getName() + " released lock for: " + filePath);
            }
        });
    }
    
    public void processDirectory(String directoryPath, FileProcessor processor) {
        try {
            Files.walk(Paths.get(directoryPath))
                .filter(Files::isRegularFile)
                .forEach(path -> processFile(path.toString(), processor));
        } catch (IOException e) {
            System.err.println("Error reading directory: " + e.getMessage());
        }
    }
    
    public void shutdown() {
        executorService.shutdown();
        try {
            if (!executorService.awaitTermination(60, TimeUnit.SECONDS)) {
                executorService.shutdownNow();
            }
        } catch (InterruptedException e) {
            executorService.shutdownNow();
        }
    }
    
    public boolean isFileBeingProcessed(String filePath) {
        ReentrantLock lock = fileLocks.get(filePath);
        return lock != null && lock.isLocked();
    }
    
    interface FileProcessor {
        void process(String filePath) throws Exception;
    }
    
    public static void main(String[] args) throws Exception {
        // Test case 1: Process multiple files concurrently
        System.out.println("=== Test Case 1: Multiple Files Processing ===");
        Task0 system1 = new Task0(3);
        String testDir1 = "test_dir1";
        createTestDirectory(testDir1, 3);
        
        FileProcessor processor1 = (filePath) -> {
            Thread.sleep(1000);
            System.out.println("Processed: " + filePath);
        };
        
        system1.processDirectory(testDir1, processor1);
        system1.shutdown();
        Thread.sleep(2000);
        
        // Test case 2: Same file accessed by multiple threads
        System.out.println("\\n=== Test Case 2: Same File Multiple Threads ===");
        Task0 system2 = new Task0(3);
        String testFile = "test_dir2/file.txt";
        createTestDirectory("test_dir2", 1);
        
        for (int i = 0; i < 3; i++) {
            system2.processFile(testFile, processor1);
        }
        system2.shutdown();
        Thread.sleep(4000);
        
        // Test case 3: Large number of files
        System.out.println("\\n=== Test Case 3: Large Number of Files ===");
        Task0 system3 = new Task0(5);
        String testDir3 = "test_dir3";
        createTestDirectory(testDir3, 10);
        
        FileProcessor processor3 = (filePath) -> {
            Thread.sleep(500);
            System.out.println("Quick process: " + filePath);
        };
        
        system3.processDirectory(testDir3, processor3);
        system3.shutdown();
        Thread.sleep(3000);
        
        // Test case 4: File content modification
        System.out.println("\\n=== Test Case 4: File Content Modification ===");
        Task0 system4 = new Task0(2);
        String testDir4 = "test_dir4";
        createTestDirectory(testDir4, 2);
        
        FileProcessor processor4 = (filePath) -> {
            List<String> lines = Files.readAllLines(Paths.get(filePath));
            lines.add("Processed by " + Thread.currentThread().getName());
            Files.write(Paths.get(filePath), lines);
            Thread.sleep(800);
        };
        
        system4.processDirectory(testDir4, processor4);
        system4.shutdown();
        Thread.sleep(2000);
        
        // Test case 5: Error handling
        System.out.println("\\n=== Test Case 5: Error Handling ===");
        Task0 system5 = new Task0(2);
        
        FileProcessor processor5 = (filePath) -> {
            if (filePath.contains("error")) {
                throw new RuntimeException("Simulated error");
            }
            Thread.sleep(500);
            System.out.println("Successfully processed: " + filePath);
        };
        
        system5.processFile("test_dir5/error_file.txt", processor5);
        system5.processFile("test_dir5/good_file.txt", processor5);
        system5.shutdown();
        Thread.sleep(2000);
        
        cleanupTestDirectories();
    }
    
    private static void createTestDirectory(String dirName, int numFiles) throws IOException {
        Path dir = Paths.get(dirName);
        Files.createDirectories(dir);
        
        for (int i = 0; i < numFiles; i++) {
            Path file = dir.resolve("file" + i + ".txt");
            Files.write(file, Arrays.asList("Test content for file " + i));
        }
    }
    
    private static void cleanupTestDirectories() throws IOException {
        for (int i = 1; i <= 5; i++) {
            String dirName = "test_dir" + i;
            Path dir = Paths.get(dirName);
            if (Files.exists(dir)) {
                Files.walk(dir)
                    .sorted(Comparator.reverseOrder())
                    .forEach(path -> {
                        try {
                            Files.delete(path);
                        } catch (IOException e) {
                            // Ignore
                        }
                    });
            }
        }
    }
}
