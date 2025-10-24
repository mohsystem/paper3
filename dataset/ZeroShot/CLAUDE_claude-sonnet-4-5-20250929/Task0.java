
import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.locks.*;

class Task0 {
    private final ConcurrentHashMap<String, ReentrantLock> fileLocks;
    private final String directoryPath;
    private final ExecutorService executorService;
    
    public Task0(String directoryPath, int threadPoolSize) {
        this.directoryPath = directoryPath;
        this.fileLocks = new ConcurrentHashMap<>();
        this.executorService = Executors.newFixedThreadPool(threadPoolSize);
    }
    
    public void processFile(String fileName, FileProcessor processor) {
        executorService.submit(() -> {
            ReentrantLock lock = fileLocks.computeIfAbsent(fileName, k -> new ReentrantLock());
            
            try {
                if (lock.tryLock(5, TimeUnit.SECONDS)) {
                    try {
                        String filePath = directoryPath + File.separator + fileName;
                        File file = new File(filePath);
                        
                        if (file.exists() && file.isFile()) {
                            processor.process(filePath);
                            System.out.println(Thread.currentThread().getName() + " processed: " + fileName);
                        } else {
                            System.out.println("File not found: " + fileName);
                        }
                    } finally {
                        lock.unlock();
                    }
                } else {
                    System.out.println(Thread.currentThread().getName() + " could not acquire lock for: " + fileName);
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                System.out.println("Thread interrupted while waiting for lock: " + fileName);
            }
        });
    }
    
    public void processAllFiles(FileProcessor processor) {
        File directory = new File(directoryPath);
        
        if (!directory.exists() || !directory.isDirectory()) {
            System.out.println("Invalid directory: " + directoryPath);
            return;
        }
        
        File[] files = directory.listFiles();
        if (files != null) {
            for (File file : files) {
                if (file.isFile()) {
                    processFile(file.getName(), processor);
                }
            }
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
    
    @FunctionalInterface
    public interface FileProcessor {
        void process(String filePath);
    }
    
    public static void main(String[] args) throws Exception {
        String testDir = "test_files";
        File dir = new File(testDir);
        if (!dir.exists()) {
            dir.mkdir();
        }
        
        for (int i = 1; i <= 5; i++) {
            File testFile = new File(testDir + File.separator + "file" + i + ".txt");
            try (PrintWriter writer = new PrintWriter(testFile)) {
                writer.println("Test content for file " + i);
            }
        }
        
        System.out.println("Test Case 1: Process multiple files concurrently");
        Task0 system1 = new Task0(testDir, 3);
        system1.processAllFiles(filePath -> {
            try {
                Thread.sleep(100);
                BufferedReader reader = new BufferedReader(new FileReader(filePath));
                String line = reader.readLine();
                reader.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
        });
        system1.shutdown();
        Thread.sleep(1000);
        
        System.out.println("\\nTest Case 2: Multiple threads accessing same file");
        Task0 system2 = new Task0(testDir, 5);
        for (int i = 0; i < 3; i++) {
            system2.processFile("file1.txt", filePath -> {
                try {
                    Thread.sleep(200);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            });
        }
        system2.shutdown();
        Thread.sleep(1000);
        
        System.out.println("\\nTest Case 3: Process with single thread pool");
        Task0 system3 = new Task0(testDir, 1);
        system3.processAllFiles(filePath -> {
            try {
                Thread.sleep(50);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });
        system3.shutdown();
        Thread.sleep(1000);
        
        System.out.println("\\nTest Case 4: Process non-existent file");
        Task0 system4 = new Task0(testDir, 2);
        system4.processFile("nonexistent.txt", filePath -> {
            System.out.println("Should not reach here");
        });
        system4.shutdown();
        Thread.sleep(500);
        
        System.out.println("\\nTest Case 5: Process with large thread pool");
        Task0 system5 = new Task0(testDir, 10);
        system5.processAllFiles(filePath -> {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });
        system5.shutdown();
        
        for (File file : dir.listFiles()) {
            file.delete();
        }
        dir.delete();
    }
}
