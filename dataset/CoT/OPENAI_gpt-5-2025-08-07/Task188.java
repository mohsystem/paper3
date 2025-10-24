// Task188 - Java implementation
// Chain-of-Through process:
// 1) Problem understanding: produce "foobar" n times using two threads alternating operations.
// 2) Security requirements: validate input range, avoid deadlocks, ensure proper synchronization, handle interruptions.
// 3) Secure coding generation: use Semaphores to strictly control alternation; ensure safe memory visibility via thread join.
// 4) Code review: no busy waiting, proper joins, final result retrieval after synchronization, bounded allocations.
// 5) Secure code output: handle invalid inputs by clamping to [0,1000]; avoid throwing in main path.

import java.util.concurrent.Semaphore;

public class Task188 {
    private static class FooBar {
        private final int n;
        private final Semaphore fooSem = new Semaphore(1);
        private final Semaphore barSem = new Semaphore(0);
        private final StringBuilder sb;

        FooBar(int n) {
            this.n = n;
            this.sb = new StringBuilder(n * 6);
        }

        public void foo() {
            try {
                for (int i = 0; i < n; i++) {
                    fooSem.acquire();
                    sb.append("foo");
                    barSem.release();
                }
            } catch (InterruptedException ie) {
                Thread.currentThread().interrupt();
            }
        }

        public void bar() {
            try {
                for (int i = 0; i < n; i++) {
                    barSem.acquire();
                    sb.append("bar");
                    fooSem.release();
                }
            } catch (InterruptedException ie) {
                Thread.currentThread().interrupt();
            }
        }

        public String result() {
            return sb.toString();
        }
    }

    // Function that accepts input as parameter and returns the output
    public static String solve(int n) {
        if (n < 0) n = 0;
        if (n > 1000) n = 1000;

        FooBar fb = new FooBar(n);

        Thread t1 = new Thread(fb::foo, "foo-thread");
        Thread t2 = new Thread(fb::bar, "bar-thread");

        t1.start();
        t2.start();

        try {
            t1.join();
            t2.join();
        } catch (InterruptedException ie) {
            Thread.currentThread().interrupt();
        }

        return fb.result();
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        int[] tests = {1, 2, 3, 5, 7};
        for (int n : tests) {
            String out = solve(n);
            System.out.println(out);
        }
    }
}