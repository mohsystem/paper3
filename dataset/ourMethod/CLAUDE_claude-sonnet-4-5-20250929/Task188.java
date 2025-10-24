
import java.util.concurrent.Semaphore;

class Task188 {
    private int n;
    private Semaphore fooSem;
    private Semaphore barSem;

    public Task188(int n) {
        if (n < 1 || n > 1000) {
            throw new IllegalArgumentException("n must be between 1 and 1000");
        }
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

    public static void main(String[] args) {
        int[] testCases = {1, 2, 3, 5, 10};
        
        for (int n : testCases) {
            System.out.print("n=" + n + ": ");
            Task188 fooBar = new Task188(n);
            StringBuilder result = new StringBuilder();
            
            Runnable printFoo = () -> result.append("foo");
            Runnable printBar = () -> result.append("bar");
            
            Thread t1 = new Thread(() -> {
                try {
                    fooBar.foo(printFoo);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
            });
            
            Thread t2 = new Thread(() -> {
                try {
                    fooBar.bar(printBar);
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
            
            System.out.println(result.toString());
        }
    }
}
