
import java.util.concurrent.Semaphore;
import java.util.function.IntConsumer;

class Task196 {
    private int n;
    private Semaphore zeroSem;
    private Semaphore evenSem;
    private Semaphore oddSem;
    
    public Task196(int n) {
        this.n = n;
        this.zeroSem = new Semaphore(1);
        this.evenSem = new Semaphore(0);
        this.oddSem = new Semaphore(0);
    }
    
    public void zero(IntConsumer printNumber) throws InterruptedException {
        for (int i = 0; i < n; i++) {
            zeroSem.acquire();
            printNumber.accept(0);
            if (i % 2 == 0) {
                oddSem.release();
            } else {
                evenSem.release();
            }
        }
    }
    
    public void even(IntConsumer printNumber) throws InterruptedException {
        for (int i = 2; i <= n; i += 2) {
            evenSem.acquire();
            printNumber.accept(i);
            zeroSem.release();
        }
    }
    
    public void odd(IntConsumer printNumber) throws InterruptedException {
        for (int i = 1; i <= n; i += 2) {
            oddSem.acquire();
            printNumber.accept(i);
            zeroSem.release();
        }
    }
    
    public static void main(String[] args) {
        testCase(2, "0102");
        testCase(5, "0102030405");
        testCase(1, "01");
        testCase(3, "010203");
        testCase(10, "01020304050607080910");
    }
    
    private static void testCase(int n, String expected) {
        Task196 zeo = new Task196(n);
        StringBuilder result = new StringBuilder();
        IntConsumer printNumber = num -> result.append(num);
        
        Thread t1 = new Thread(() -> {
            try {
                zeo.zero(printNumber);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });
        
        Thread t2 = new Thread(() -> {
            try {
                zeo.even(printNumber);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });
        
        Thread t3 = new Thread(() -> {
            try {
                zeo.odd(printNumber);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });
        
        t1.start();
        t2.start();
        t3.start();
        
        try {
            t1.join();
            t2.join();
            t3.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        
        System.out.println("Test n=" + n + ": " + (result.toString().equals(expected) ? "PASS" : "FAIL") + " (Expected: " + expected + ", Got: " + result + ")");
    }
}
