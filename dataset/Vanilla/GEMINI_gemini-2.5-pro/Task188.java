import java.util.concurrent.Semaphore;

class FooBar {
    private int n;
    private Semaphore fooSem;
    private Semaphore barSem;

    public FooBar(int n) {
        this.n = n;
        // fooSem starts with 1 permit, allowing foo() to run first.
        this.fooSem = new Semaphore(1);
        // barSem starts with 0 permits, making bar() wait.
        this.barSem = new Semaphore(0);
    }

    public void foo(Runnable printFoo) throws InterruptedException {
        for (int i = 0; i < n; i++) {
            fooSem.acquire();
            printFoo.run();
            barSem.release();
        }
    }

    public void bar(Runnable printBar) throws InterruptedException {
        for (int i = 0; i < n; i++) {
            barSem.acquire();
            printBar.run();
            fooSem.release();
        }
    }
}

public class Task188 {
    public static void main(String[] args) {
        int[] testCases = {1, 2, 5, 10, 100};
        for (int n : testCases) {
            runTest(n);
            System.out.println(); // Newline after each test case
        }
    }

    public static void runTest(int n) {
        if (n <= 0) return;
        FooBar fooBar = new FooBar(n);
        Runnable printFoo = () -> System.out.print("foo");
        Runnable printBar = () -> System.out.print("bar");

        Thread threadA = new Thread(() -> {
            try {
                fooBar.foo(printFoo);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        Thread threadB = new Thread(() -> {
            try {
                fooBar.bar(printBar);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        threadA.start();
        threadB.start();

        try {
            threadA.join();
            threadB.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
}