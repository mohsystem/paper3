
import java.util.concurrent.Semaphore;
import java.util.function.IntConsumer;

class Task189 {
    private int n;
    private Semaphore zeroSem;
    private Semaphore evenSem;
    private Semaphore oddSem;
    
    public Task189(int n) {
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
        testCase(4, "01020304");
    }
    
    private static void testCase(int n, String expected) {
        Task189 zeroEvenOdd = new Task189(n);
        StringBuilder result = new StringBuilder();
        
        IntConsumer printNumber = num -> {
            synchronized (result) {
                result.append(num);
            }
        };
        
        Thread threadA = new Thread(() -> {
            try {
                zeroEvenOdd.zero(printNumber);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });
        
        Thread threadB = new Thread(() -> {
            try {
                zeroEvenOdd.even(printNumber);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });
        
        Thread threadC = new Thread(() -> {
            try {
                zeroEvenOdd.odd(printNumber);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });
        
        threadA.start();
        threadB.start();
        threadC.start();
        
        try {
            threadA.join();
            threadB.join();
            threadC.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        
        System.out.println("Input: n = " + n);
        System.out.println("Expected: " + expected);
        System.out.println("Output: " + result.toString());
        System.out.println("Test " + (result.toString().equals(expected) ? "PASSED" : "FAILED"));
        System.out.println();
    }
}
