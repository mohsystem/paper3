import java.util.concurrent.Semaphore;

public class Task188 {

    private int n;
    private Semaphore fooSem = new Semaphore(1);
    private Semaphore barSem = new Semaphore(0);

    public Task188(int n) {
        this.n = n;
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

    private static void runTest(int n) {
        System.out.println("Running test for n = " + n);
        if (n <= 0) {
            System.out.println("Output: \"\"");
            System.out.println("--------------------");
            return;
        }

        Task188 fooBar = new Task188(n);
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

        System.out.print("Output: \"");
        threadA.start();
        threadB.start();

        try {
            threadA.join();
            threadB.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        System.out.println("\"");
        System.out.println("--------------------");
    }

    public static void main(String[] args) {
        int[] testCases = {1, 2, 5, 10, 0};
        for (int n : testCases) {
            runTest(n);
        }
    }
}