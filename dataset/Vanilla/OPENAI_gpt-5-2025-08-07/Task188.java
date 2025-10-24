import java.util.concurrent.Semaphore;

public class Task188 {

    static class FooBar {
        private final int n;
        private final Semaphore fooSem = new Semaphore(1);
        private final Semaphore barSem = new Semaphore(0);
        private final StringBuilder sb = new StringBuilder();

        public FooBar(int n) {
            this.n = n;
        }

        public void foo() {
            for (int i = 0; i < n; i++) {
                try {
                    fooSem.acquire();
                    sb.append("foo");
                    barSem.release();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    return;
                }
            }
        }

        public void bar() {
            for (int i = 0; i < n; i++) {
                try {
                    barSem.acquire();
                    sb.append("bar");
                    fooSem.release();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    return;
                }
            }
        }

        public String getOutput() {
            return sb.toString();
        }
    }

    public static String solve(int n) {
        FooBar fb = new FooBar(n);
        Thread t1 = new Thread(fb::foo);
        Thread t2 = new Thread(fb::bar);
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

    public static void main(String[] args) {
        int[] tests = {1, 2, 3, 4, 5};
        for (int n : tests) {
            System.out.println(solve(n));
        }
    }
}