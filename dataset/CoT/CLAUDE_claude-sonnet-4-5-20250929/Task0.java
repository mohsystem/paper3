
import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.locks.*;

public class Task0 {
    private final ConcurrentHashMap<String, ReentrantLock> fileLocks;
    private final ExecutorService executorService;
    private final String baseDirectory;
    
    public Task0(String baseDirectory, int threadPoolSize) {
        this.fileLocks = new ConcurrentHashMap<>();
        this.executorService = Executors.newFixedThreadPool(threadPoolSize);
        this.baseDirectory = validateAndNormalizePath(baseDirectory);
    }
    
    private String validateAndNormalizePath(String path) {
        try {
            Path normalizedPath = Paths.get(path).toRealPath();
            return normalizedPath.toString();
        } catch (IOException e) {
            throw new IllegalArgumentException("Invalid directory path: " + path);
        }
    }
    
    private boolean isPathSafe(String filePath) {
        try {
            Path requestedPath = Paths.get(filePath).toRealPath();
            Path basePath = Paths.get(baseDirectory).toRealPath();
            return requestedPath.startsWith(basePath);
        } catch (IOException e) {
            return false;
        }
    }
    
    public void processFile(String fileName) {
        executorService.submit(() -> {
            String fullPath = Paths.get(baseDirectory, fileName).toString();
            
            if (!isPathSafe(fullPath)) {
                System.err.println("Security violation: Path traversal attempt detected for " + fileName);
                return;
            }
            
            ReentrantLock lock = fileLocks.computeIfAbsent(fullPath, k -> new ReentrantLock());
            
            try {
                if (lock.tryLock(5, TimeUnit.SECONDS)) {
                    try {
                        processFileContent(fullPath);
                    } finally {
                        lock.unlock();
                    }
                } else {
                    System.err.println("Timeout: Could not acquire lock for " + fileName);
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                System.err.println("Thread interrupted while waiting for lock: " + fileName);
            }
        });
    }
    
    private void processFileContent(String filePath) {
        try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
            System.out.println("Thread " + Thread.currentThread().getId() + " processing: " + filePath);
            String line;
            int lineCount = 0;
            while ((line = reader.readLine()) != null) {
                lineCount++;
            }
            System.out.println("Thread " + Thread.currentThread().getId() + " completed: " + filePath + " (" + lineCount + " lines)");
        } catch (IOException e) {
            System.err.println("Error processing file " + filePath + ": " + e.getMessage());
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
            Thread.currentThread().interrupt();
        }
    }
    
    public static void main(String[] args) {
        try {
            String testDir = System.getProperty("java.io.tmpdir") + File.separator + "test_concurrent_files";
            Files.createDirectories(Paths.get(testDir));
            
            for (int i = 1; i <= 5; i++) {
                Path testFile = Paths.get(testDir, "test_file_" + i + ".txt");
                try (BufferedWriter writer = Files.newBufferedWriter(testFile)) {
                    for (int j = 0; j < 100; j++) {
                        writer.write("Test data line " + j + " in file " + i + "\\n");
                    }
                }
            }
            
            Task0 processor = new Task0(testDir, 3);
            
            System.out.println("Test Case 1: Processing multiple files concurrently");
            processor.processFile("test_file_1.txt");
            processor.processFile("test_file_2.txt");
            processor.processFile("test_file_3.txt");
            
            System.out.println("\\nTest Case 2: Multiple threads attempting same file");
            processor.processFile("test_file_1.txt");
            processor.processFile("test_file_1.txt");
            
            System.out.println("\\nTest Case 3: Processing remaining files");
            processor.processFile("test_file_4.txt");
            processor.processFile("test_file_5.txt");
            
            System.out.println("\\nTest Case 4: Path traversal attack attempt");
            processor.processFile("../../../etc/passwd");
            
            System.out.println("\\nTest Case 5: Non-existent file");
            processor.processFile("non_existent_file.txt");
            
            processor.shutdown();
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
