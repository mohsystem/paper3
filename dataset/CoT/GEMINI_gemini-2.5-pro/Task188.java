import java.util.concurrent.Semaphore;

class Task188 {
    // Inner class to handle the FooBar logic
    static class FooBar {
        private int n;
        private Semaphore fooSem;
        private Semaphore barSem;

        public FooBar(int n) {
            this.n = n;
            // fooSem is initialized to 1 so that foo() can run first.
            this.fooSem = new Semaphore(1);
            // barSem is initialized to 0 so that bar() has to wait.
            this.barSem = new Semaphore(0);
        }

        public void foo(Runnable printFoo) throws InterruptedException {
            for (int i = 0; i < n; i++) {
                // Acquire the foo semaphore. This will block if the permit is not available.
                fooSem.acquire();
                // printFoo.run() outputs "foo". Do not change or remove this line.
                printFoo.run();
                // Release the bar semaphore, allowing the bar() method to proceed.
                barSem.release();
            }
        }

        public void bar(Runnable printBar) throws InterruptedException {
            for (int i = 0; i < n; i++) {
                // Acquire the bar semaphore. This will block until foo() releases it.
                barSem.acquire();
                // printBar.run() outputs "bar". Do not change or remove this line.
                printBar.run();
                // Release the foo semaphore, allowing the foo() method to proceed for the next iteration.
                fooSem.release();
            }
        }
    }

    public static void runTest(int n) throws InterruptedException {
        System.out.println("Test Case (n = " + n + "):");
        final FooBar fooBar = new FooBar(n);
        
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

        threadA.join();
        threadB.join();
        System.out.println("\n");
    }

    public static void main(String[] args) {
        try {
            runTest(1);
            runTest(2);
            runTest(5);
            runTest(10);
            runTest(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
            Thread.currentThread().interrupt();
        }
    }
}