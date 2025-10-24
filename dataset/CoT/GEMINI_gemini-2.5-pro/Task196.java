import java.util.function.IntConsumer;

class Task196 {
    private int n;
    private volatile int turn = 0; // 0 for zero, 1 for odd, 2 for even
    private final Object lock = new Object();

    public Task196(int n) {
        this.n = n;
    }

    // printNumber.accept(x) outputs x to the console.
    public void zero(IntConsumer printNumber) throws InterruptedException {
        for (int i = 1; i <= n; i++) {
            synchronized (lock) {
                while (turn != 0) {
                    lock.wait();
                }
                printNumber.accept(0);
                if (i % 2 != 0) {
                    turn = 1; // Next is odd
                } else {
                    turn = 2; // Next is even
                }
                lock.notifyAll();
            }
        }
    }

    public void even(IntConsumer printNumber) throws InterruptedException {
        for (int i = 2; i <= n; i += 2) {
            synchronized (lock) {
                while (turn != 2) {
                    lock.wait();
                }
                printNumber.accept(i);
                turn = 0;
                lock.notifyAll();
            }
        }
    }

    public void odd(IntConsumer printNumber) throws InterruptedException {
        for (int i = 1; i <= n; i += 2) {
            synchronized (lock) {
                while (turn != 1) {
                    lock.wait();
                }
                printNumber.accept(i);
                turn = 0;
                lock.notifyAll();
            }
        }
    }
    
    public static void runTest(int n) {
        System.out.println("Test Case for n = " + n);
        System.out.print("Output: ");
        
        Task196 zeroEvenOdd = new Task196(n);
        IntConsumer printNumber = (x) -> System.out.print(x);

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
        System.out.println("\n");
    }

    public static void main(String[] args) {
        runTest(2);
        runTest(5);
        runTest(1);
        runTest(6);
        runTest(10);
    }
}