
import java.util.concurrent.Semaphore;
import java.util.function.IntConsumer;

class Task189 {
    private int n;
    private Semaphore zeroSem;
    private Semaphore evenSem;
    private Semaphore oddSem;
    private int current;

    public Task189(int n) {
        this.n = n;
        this.zeroSem = new Semaphore(1);
        this.evenSem = new Semaphore(0);
        this.oddSem = new Semaphore(0);
        this.current = 1;
    }

    public void zero(IntConsumer printNumber) throws InterruptedException {
        for (int i = 0; i < n; i++) {
            zeroSem.acquire();
            printNumber.accept(0);
            if (current % 2 == 1) {
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
            current++;
            zeroSem.release();
        }
    }

    public void odd(IntConsumer printNumber) throws InterruptedException {
        for (int i = 1; i <= n; i += 2) {
            oddSem.acquire();
            printNumber.accept(i);
            current++;
            zeroSem.release();
        }
    }

    public static void main(String[] args) {
        // Test case 1
        testCase(2, "0102");
        
        // Test case 2
        testCase(5, "0102030405");
        
        // Test case 3
        testCase(1, "01");
        
        // Test case 4
        testCase(3, "010203");
        
        // Test case 5
        testCase(10, "01020304050607080910");
    }

    private static void testCase(int n, String expected) {
        Task189 zeo = new Task189(n);
        StringBuilder result = new StringBuilder();
        IntConsumer printNumber = result::append;

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

        System.out.println("Input: n = " + n);
        System.out.println("Output: \\"" + result.toString() + "\\"");
        System.out.println("Expected: \\"" + expected + "\\"");
        System.out.println("Pass: " + result.toString().equals(expected));
        System.out.println();
    }
}
