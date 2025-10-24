import java.util.Arrays;

public final class Task149 {

    // Immutable result type for dequeue/peek
    public static final class DequeueResult {
        public final boolean success;
        public final int value;

        public DequeueResult(boolean success, int value) {
            this.success = success;
            this.value = value;
        }

        @Override
        public String toString() {
            if (!success) {
                return "Result{success=false}";
            }
            return "Result{success=true, value=" + value + "}";
        }
    }

    public static final class IntQueue {
        private static final int MAX_CAPACITY = 1_000_000;
        private final int[] buffer;
        private final int capacity;
        private int head; // index of current front
        private int tail; // index of next write
        private int size;

        public IntQueue(int capacity) {
            if (capacity < 1 || capacity > MAX_CAPACITY) {
                throw new IllegalArgumentException("Capacity must be between 1 and " + MAX_CAPACITY);
            }
            this.capacity = capacity;
            this.buffer = new int[capacity];
            this.head = 0;
            this.tail = 0;
            this.size = 0;
        }

        public int size() {
            return size;
        }

        public int capacity() {
            return capacity;
        }

        public boolean isEmpty() {
            return size == 0;
        }

        public boolean isFull() {
            return size == capacity;
        }

        // Enqueue returns true on success; false if full
        public boolean enqueue(int value) {
            if (isFull()) {
                return false;
            }
            buffer[tail] = value;
            tail = (tail + 1) % capacity;
            size++;
            return true;
        }

        // Dequeue returns {false, 0} if empty
        public DequeueResult dequeue() {
            if (isEmpty()) {
                return new DequeueResult(false, 0);
            }
            int v = buffer[head];
            head = (head + 1) % capacity;
            size--;
            return new DequeueResult(true, v);
        }

        // Peek returns {false, 0} if empty
        public DequeueResult peek() {
            if (isEmpty()) {
                return new DequeueResult(false, 0);
            }
            return new DequeueResult(true, buffer[head]);
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("IntQueue{size=").append(size).append(", capacity=").append(capacity).append(", data=[");
            for (int i = 0; i < size; i++) {
                int idx = (head + i) % capacity;
                sb.append(buffer[idx]);
                if (i + 1 < size) sb.append(", ");
            }
            sb.append("]}");
            return sb.toString();
        }
    }

    private static void test1() {
        System.out.println("TEST 1: Basic enqueue/dequeue");
        IntQueue q = new IntQueue(3);
        System.out.println("enqueue 1 -> " + q.enqueue(1));
        System.out.println("enqueue 2 -> " + q.enqueue(2));
        System.out.println("enqueue 3 -> " + q.enqueue(3));
        System.out.println("queue: " + q);
        System.out.println("peek -> " + q.peek());
        System.out.println("dequeue -> " + q.dequeue());
        System.out.println("dequeue -> " + q.dequeue());
        System.out.println("dequeue -> " + q.dequeue());
        System.out.println("dequeue (empty) -> " + q.dequeue());
        System.out.println("queue: " + q);
    }

    private static void test2() {
        System.out.println("TEST 2: Overflow handling");
        IntQueue q = new IntQueue(2);
        System.out.println("enqueue 10 -> " + q.enqueue(10));
        System.out.println("enqueue 20 -> " + q.enqueue(20));
        System.out.println("enqueue 30 (should fail) -> " + q.enqueue(30));
        System.out.println("peek -> " + q.peek());
        System.out.println("queue: " + q);
    }

    private static void test3() {
        System.out.println("TEST 3: Peek on empty");
        IntQueue q = new IntQueue(5);
        System.out.println("peek (empty) -> " + q.peek());
        System.out.println("enqueue 7 -> " + q.enqueue(7));
        System.out.println("peek -> " + q.peek());
        System.out.println("dequeue -> " + q.dequeue());
        System.out.println("peek (empty) -> " + q.peek());
    }

    private static void test4() {
        System.out.println("TEST 4: Wrap-around behavior");
        IntQueue q = new IntQueue(3);
        System.out.println("enqueue 1 -> " + q.enqueue(1));
        System.out.println("enqueue 2 -> " + q.enqueue(2));
        System.out.println("dequeue -> " + q.dequeue());
        System.out.println("enqueue 3 -> " + q.enqueue(3));
        System.out.println("enqueue 4 -> " + q.enqueue(4));
        System.out.println("isFull -> " + q.isFull());
        System.out.println("queue: " + q);
        System.out.println("dequeue -> " + q.dequeue());
        System.out.println("dequeue -> " + q.dequeue());
        System.out.println("dequeue -> " + q.dequeue());
        System.out.println("isEmpty -> " + q.isEmpty());
    }

    private static void test5() {
        System.out.println("TEST 5: Capacity 1 edge case");
        IntQueue q = new IntQueue(1);
        System.out.println("enqueue 42 -> " + q.enqueue(42));
        System.out.println("enqueue 99 (should fail) -> " + q.enqueue(99));
        System.out.println("peek -> " + q.peek());
        System.out.println("dequeue -> " + q.dequeue());
        System.out.println("dequeue (empty) -> " + q.dequeue());
    }

    public static void main(String[] args) {
        try {
            test1();
            System.out.println();
            test2();
            System.out.println();
            test3();
            System.out.println();
            test4();
            System.out.println();
            test5();
        } catch (RuntimeException ex) {
            // Fail closed with clear error
            System.out.println("Error: " + ex.getMessage());
        }
    }
}