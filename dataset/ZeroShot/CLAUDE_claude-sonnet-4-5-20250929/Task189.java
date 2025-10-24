
import java.util.concurrent.Semaphore;
import java.util.function.IntConsumer;

class ZeroEvenOdd {
    private int n;
    private Semaphore zeroSem;
    private Semaphore evenSem;
    private Semaphore oddSem;
    
    public ZeroEvenOdd(int n) {
        this.n = n;
        this.zeroSem = new Semaphore(1);
        this.evenSem = new Semaphore(0);
        this.oddSem = new Semaphore(0);
    }

    public void zero(IntConsumer printNumber) throws InterruptedException {
        for (int i = 1; i <= n; i++) {
            zeroSem.acquire();
            printNumber.accept(0);
            if (i % 2 == 1) {
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
}

class Task189 {
    public static void main(String[] args) {
        // Test case 1: n = 2
        System.out.print("Test 1 (n=2): ");
        runTest(2);
        
        // Test case 2: n = 5
        System.out.print("Test 2 (n=5): ");
        runTest(5);
        
        // Test case 3: n = 1
        System.out.print("Test 3 (n=1): ");
        runTest(1);
        
        // Test case 4: n = 10
        System.out.print("Test 4 (n=10): ");
        runTest(10);
        
        // Test case 5: n = 7
        System.out.print("Test 5 (n=7): ");
        runTest(7);
    }
    
    private static void runTest(int n) {
        ZeroEvenOdd zeo = new ZeroEvenOdd(n);
        StringBuilder result = new StringBuilder();
        IntConsumer printNumber = (num) -> result.append(num);
        
        Thread t1 = new Thread(() -> {
            try {
                zeo.zero(printNumber);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        });
        
        Thread t2 = new Thread(() -> {
            try {
                zeo.even(printNumber);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        });
        
        Thread t3 = new Thread(() -> {
            try {
                zeo.odd(printNumber);
            } catch (InterruptedException e) {
                e.printStackTrace();
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
            e.printStackTrace();
        }
        
        System.out.println(result.toString());
    }
}
