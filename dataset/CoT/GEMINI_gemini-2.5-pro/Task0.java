import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.Queue;
import java.util.List;
import java.util.ArrayList;

class FileProcessor implements Runnable {
    private Queue<String> fileQueue;
    private String threadName;

    public FileProcessor(Queue<String> fileQueue, int id) {
        this.fileQueue = fileQueue;
        this.threadName = "Thread-" + id;
    }

    @Override
    public void run() {
        while (true) {
            String fileName = fileQueue.poll();
            if (fileName == null) {
                // No more files to process
                break;
            }
            System.out.println(threadName + " started processing " + fileName);
            try {
                // Simulate file processing time
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                System.err.println(threadName + " was interrupted.");
            }
            System.out.println(threadName + " finished processing " + fileName);
        }
    }
}

public class Task0 {

    public static void processFiles(String[] files, int numThreads) {
        System.out.println("--- Java Concurrent File Processing ---");
        
        Queue<String> fileQueue = new ConcurrentLinkedQueue<>();
        for (String file : files) {
            fileQueue.offer(file);
        }

        List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < numThreads; i++) {
            Thread thread = new Thread(new FileProcessor(fileQueue, i + 1));
            threads.add(thread);
            thread.start();
        }

        for (Thread thread : threads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                System.err.println("Main thread interrupted while waiting.");
            }
        }
        System.out.println("All files have been processed.\n");
    }

    public static void main(String[] args) {
        // Test Case 1
        processFiles(new String[]{"file1.log", "file2.data", "archive.zip", "image.jpg", "report.docx"}, 3);
        
        // Test Case 2
        processFiles(new String[]{"document.pdf", "data.csv"}, 2);
        
        // Test Case 3
        processFiles(new String[]{"single_file.txt"}, 4);
        
        // Test Case 4
        processFiles(new String[]{"audio.mp3", "video.mp4", "backup.sql", "config.ini", "style.css", "script.js"}, 4);

        // Test Case 5
        processFiles(new String[]{}, 3); // No files
    }
}