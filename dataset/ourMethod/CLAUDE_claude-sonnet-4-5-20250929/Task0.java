
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.MessageDigest;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.locks.ReentrantLock;

public class Task0 {
    private static final String MAGIC = "PROC";
    private static final int VERSION = 1;
    private static final int MAX_PATH_LENGTH = 4096;
    private static final int MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB
    
    private final Path baseDirectory;
    private final ConcurrentHashMap<String, ReentrantLock> fileLocks;
    private final ExecutorService executorService;
    
    public Task0(String basePath) throws IOException {
        if (basePath == null || basePath.trim().isEmpty()) {
            throw new IllegalArgumentException("Base path cannot be null or empty");
        }
        
        Path normalized = Paths.get(basePath).toAbsolutePath().normalize();
        if (!Files.exists(normalized)) {
            throw new IllegalArgumentException("Base directory does not exist");
        }
        if (!Files.isDirectory(normalized)) {
            throw new IllegalArgumentException("Base path is not a directory");
        }
        
        this.baseDirectory = normalized;
        this.fileLocks = new ConcurrentHashMap<>();
        this.executorService = Executors.newFixedThreadPool(4);
    }
    
    private Path validateAndResolvePath(String filename) throws IOException {
        if (filename == null || filename.trim().isEmpty()) {
            throw new IllegalArgumentException("Filename cannot be null or empty");
        }
        
        if (filename.length() > MAX_PATH_LENGTH) {
            throw new IllegalArgumentException("Filename exceeds maximum length");
        }
        
        if (filename.contains("..") || filename.contains("~")) {
            throw new SecurityException("Path traversal detected");
        }
        
        Path resolved = baseDirectory.resolve(filename).toAbsolutePath().normalize();
        
        if (!resolved.startsWith(baseDirectory)) {
            throw new SecurityException("Resolved path outside base directory");
        }
        
        if (Files.exists(resolved)) {
            if (!Files.isRegularFile(resolved)) {
                throw new IllegalArgumentException("Path is not a regular file");
            }
            if (Files.isSymbolicLink(resolved)) {
                throw new SecurityException("Symbolic links are not allowed");
            }
        }
        
        return resolved;
    }
    
    public Future<String> processFile(String filename) {
        return executorService.submit(() -> {
            Path filePath = validateAndResolvePath(filename);
            String canonicalKey = filePath.toString();
            
            ReentrantLock lock = fileLocks.computeIfAbsent(canonicalKey, k -> new ReentrantLock());
            
            lock.lock();
            try {
                if (!Files.exists(filePath)) {
                    return "File not found: " + filename;
                }
                
                long fileSize = Files.size(filePath);
                if (fileSize > MAX_FILE_SIZE) {
                    return "File too large: " + filename;
                }
                
                Path tempPath = Files.createTempFile(baseDirectory, ".proc_", ".tmp");
                
                try {
                    StringBuilder content = new StringBuilder();
                    content.append(MAGIC);
                    content.append(VERSION);
                    content.append("\\n");
                    
                    try (BufferedReader reader = Files.newBufferedReader(filePath, StandardCharsets.UTF_8)) {
                        String line;
                        while ((line = reader.readLine()) != null) {
                            if (line.length() > 10000) {
                                line = line.substring(0, 10000);
                            }
                            content.append(line.toUpperCase()).append("\\n");
                        }
                    }
                    
                    try (BufferedWriter writer = Files.newBufferedWriter(tempPath, StandardCharsets.UTF_8)) {
                        writer.write(content.toString());
                        writer.flush();
                    }
                    
                    Path outputPath = baseDirectory.resolve(filename + ".processed");
                    Files.move(tempPath, outputPath, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
                    
                    return "Processed: " + filename;
                } catch (Exception e) {
                    Files.deleteIfExists(tempPath);
                    throw e;
                }
            } finally {
                lock.unlock();
            }
        });
    }
    
    public void shutdown() throws InterruptedException {
        executorService.shutdown();
        if (!executorService.awaitTermination(60, TimeUnit.SECONDS)) {
            executorService.shutdownNow();
        }
    }
    
    public static void main(String[] args) {
        try {
            Path testDir = Files.createTempDirectory("fileproc_test");
            
            Files.write(testDir.resolve("file1.txt"), "hello world".getBytes(StandardCharsets.UTF_8));
            Files.write(testDir.resolve("file2.txt"), "test data".getBytes(StandardCharsets.UTF_8));
            Files.write(testDir.resolve("file3.txt"), "sample content".getBytes(StandardCharsets.UTF_8));
            Files.write(testDir.resolve("file4.txt"), "another file".getBytes(StandardCharsets.UTF_8));
            Files.write(testDir.resolve("file5.txt"), "final test".getBytes(StandardCharsets.UTF_8));
            
            Task0 processor = new Task0(testDir.toString());
            
            List<Future<String>> futures = new ArrayList<>();
            futures.add(processor.processFile("file1.txt"));
            futures.add(processor.processFile("file2.txt"));
            futures.add(processor.processFile("file3.txt"));
            futures.add(processor.processFile("file4.txt"));
            futures.add(processor.processFile("file5.txt"));
            
            for (Future<String> future : futures) {
                System.out.println(future.get());
            }
            
            processor.shutdown();
            
            Files.walk(testDir)
                .sorted(Comparator.reverseOrder())
                .forEach(path -> {
                    try { Files.deleteIfExists(path); } catch (IOException e) {}
                });
                
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
