
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class Task92 {
    private int currentCounter = 0;
    private int maxCounter;
    private Lock lock = new ReentrantLock();
    
    public Task92(int maxCounter) {
        this.maxCounter = maxCounter;
    }
    
    public void accessSharedResource(String threadName) {
        if (currentCounter <= maxCounter) {
            lock.lock();
            try {
                if (currentCounter <= maxCounter) {
                    currentCounter++;
                    System.out.println(threadName + " is accessing currentCounter: " + currentCounter);
                }
            } finally {
                lock.unlock();
            }
        }
    }
    
    public int getCurrentCounter() {
        return currentCounter;
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic test with 5 threads and max counter 10
        System.out.println("Test Case 1:");
        Task92 test1 = new Task92(10);
        for (int i = 1; i <= 5; i++) {
            final int threadNum = i;
            new Thread(() -> test1.accessSharedResource("Thread-" + threadNum)).start();
        }
        
        try { Thread.sleep(1000); } catch (InterruptedException e) {}
        
        // Test case 2: Multiple threads with smaller max counter
        System.out.println("\\nTest Case 2:");
        Task92 test2 = new Task92(3);
        for (int i = 1; i <= 5; i++) {
            final int threadNum = i;
            new Thread(() -> test2.accessSharedResource("Worker-" + threadNum)).start();
        }
        
        try { Thread.sleep(1000); } catch (InterruptedException e) {}
        
        // Test case 3: Single thread
        System.out.println("\\nTest Case 3:");
        Task92 test3 = new Task92(1);
        new Thread(() -> test3.accessSharedResource("SingleThread")).start();
        
        try { Thread.sleep(1000); } catch (InterruptedException e) {}
        
        // Test case 4: Max counter is 0
        System.out.println("\\nTest Case 4:");
        Task92 test4 = new Task92(0);
        for (int i = 1; i <= 3; i++) {
            final int threadNum = i;
            new Thread(() -> test4.accessSharedResource("Thread-" + threadNum)).start();
        }
        
        try { Thread.sleep(1000); } catch (InterruptedException e) {}
        
        // Test case 5: Many threads with moderate max counter
        System.out.println("\\nTest Case 5:");
        Task92 test5 = new Task92(7);
        for (int i = 1; i <= 10; i++) {
            final int threadNum = i;
            new Thread(() -> test5.accessSharedResource("Task-" + threadNum)).start();
        }
        
        try { Thread.sleep(1000); } catch (InterruptedException e) {}
    }
}
