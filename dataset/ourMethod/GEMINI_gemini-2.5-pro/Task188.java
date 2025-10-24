import java.util.concurrent.Semaphore;

public class Task188 {

    static class FooBar {
        private int n;
        private final Semaphore fooSem;
        private final Semaphore barSem;

        public FooBar(int n) {
            this.n = n;
            this.fooSem = new Semaphore(1);
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

    private static void runTest(int n) {
        System.out.println("Test with n = " + n + ":");
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
        System.out.println("\n");
    }

    public static void main(String[] args) {
        runTest(1);
        runTest(2);
        runTest(5);
        runTest(10);
        runTest(0); // Edge case
    }
}