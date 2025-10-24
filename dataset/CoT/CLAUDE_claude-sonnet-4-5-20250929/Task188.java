
import java.util.concurrent.Semaphore;

class Task188 {
    private int n;
    private Semaphore fooSem;
    private Semaphore barSem;

    public Task188(int n) {
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
        // Test case 1
        System.out.println("Test case 1:");
        testFooBar(1);
        
        // Test case 2
        System.out.println("\\nTest case 2:");
        testFooBar(2);
        
        // Test case 3
        System.out.println("\\nTest case 3:");
        testFooBar(5);
        
        // Test case 4
        System.out.println("\\nTest case 4:");
        testFooBar(10);
        
        // Test case 5
        System.out.println("\\nTest case 5:");
        testFooBar(3);
    }

    private static void testFooBar(int n) {
        Task188 fooBar = new Task188(n);
        
        Thread threadA = new Thread(() -> {
            try {
                fooBar.foo(() -> System.out.print("foo"));
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });
        
        Thread threadB = new Thread(() -> {
            try {
                fooBar.bar(() -> System.out.print("bar"));
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
        System.out.println();
    }
}
