import java.util.concurrent.Semaphore;

public class Task188 {

    // Main method with 5 test cases
    public static void main(String[] args) {
        int[] tests = {1, 2, 5, 10, 0};
        for (int n : tests) {
            try {
                String res = generateFoobar(n);
                System.out.println("n=" + n + " -> " + res);
            } catch (IllegalArgumentException e) {
                System.out.println("n=" + n + " -> error: " + e.getMessage());
            }
        }
    }

    public static String generateFoobar(int n) {
        if (n < 1 || n > 1000) {
            throw new IllegalArgumentException("n must be in [1, 1000]");
        }
        final SafeAppender out = new SafeAppender();
        FooBar fb = new FooBar(n);

        Thread t1 = new Thread(() -> {
            try {
                fb.foo(() -> out.append("foo"));
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        Thread t2 = new Thread(() -> {
            try {
                fb.bar(() -> out.append("bar"));
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        t1.start();
        t2.start();
        try {
            t1.join();
            t2.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        return out.get();
    }

    // Thread-safe appender
    private static final class SafeAppender {
        private final StringBuilder sb = new StringBuilder();
        public synchronized void append(String s) {
            sb.append(s);
        }
        public synchronized String get() {
            return sb.toString();
        }
    }

    // FooBar using semaphores to alternate
    private static final class FooBar {
        private final int n;
        private final Semaphore fooSem = new Semaphore(1);
        private final Semaphore barSem = new Semaphore(0);

        FooBar(int n) {
            this.n = n;
        }

        // Returns number of iterations completed
        public int foo(Runnable printFoo) throws InterruptedException {
            for (int i = 0; i < n; i++) {
                fooSem.acquire();
                printFoo.run();
                barSem.release();
            }
            return n;
        }

        // Returns number of iterations completed
        public int bar(Runnable printBar) throws InterruptedException {
            for (int i = 0; i < n; i++) {
                barSem.acquire();
                printBar.run();
                fooSem.release();
            }
            return n;
        }
    }
}