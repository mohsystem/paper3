import java.util.concurrent.Semaphore;

public class Task188 {

    // Runs the FooBar threading task and returns the concatenated result
    public static String runFooBar(int n) {
        if (n < 1 || n > 1000) {
            return "";
        }
        FooBar fb = new FooBar(n);
        Thread t1 = new Thread(() -> {
            try {
                fb.foo();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });
        Thread t2 = new Thread(() -> {
            try {
                fb.bar();
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
        return fb.getOutput();
    }

    // Inner class implementing the FooBar logic using semaphores
    static final class FooBar {
        private final int n;
        private final Semaphore fooSem = new Semaphore(1);
        private final Semaphore barSem = new Semaphore(0);
        private final StringBuilder sb = new StringBuilder();

        FooBar(int n) {
            this.n = n;
        }

        public void foo() throws InterruptedException {
            for (int i = 0; i < n; i++) {
                fooSem.acquire();
                sb.append("foo");
                barSem.release();
            }
        }

        public void bar() throws InterruptedException {
            for (int i = 0; i < n; i++) {
                barSem.acquire();
                sb.append("bar");
                fooSem.release();
            }
        }

        public String getOutput() {
            return sb.toString();
        }
    }

    public static void main(String[] args) {
        int[] tests = {1, 2, 3, 5, 10};
        for (int n : tests) {
            String out = runFooBar(n);
            System.out.println(out);
        }
    }
}