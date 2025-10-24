import java.util.OptionalInt;

public class Task149 {
    // A simple integer queue implemented as a circular buffer with dynamic resizing.
    static class IntQueue {
        private int[] data;
        private int head;
        private int tail;
        private int size;

        public IntQueue() {
            this(16);
        }

        public IntQueue(int initialCapacity) {
            if (initialCapacity <= 0) {
                throw new IllegalArgumentException("Initial capacity must be positive.");
            }
            data = new int[initialCapacity];
            head = 0;
            tail = 0;
            size = 0;
        }

        public boolean enqueue(int value) {
            ensureCapacityForOneMore();
            data[tail] = value;
            tail = (tail + 1) % data.length;
            size++;
            return true;
        }

        public OptionalInt dequeue() {
            if (size == 0) {
                return OptionalInt.empty();
            }
            int val = data[head];
            data[head] = 0; // clear reference
            head = (head + 1) % data.length;
            size--;
            return OptionalInt.of(val);
        }

        public OptionalInt peek() {
            if (size == 0) {
                return OptionalInt.empty();
            }
            return OptionalInt.of(data[head]);
        }

        public int size() {
            return size;
        }

        public boolean isEmpty() {
            return size == 0;
        }

        private void ensureCapacityForOneMore() {
            if (size < data.length) return;
            int current = data.length;
            int grown = safeGrow(current);
            int[] newData = new int[grown];
            // Copy elements in order from head to tail
            if (head <= tail) {
                System.arraycopy(data, head, newData, 0, size);
            } else {
                int firstPart = data.length - head;
                System.arraycopy(data, head, newData, 0, firstPart);
                System.arraycopy(data, 0, newData, firstPart, tail);
            }
            data = newData;
            head = 0;
            tail = size;
        }

        private int safeGrow(int current) {
            // Double capacity, with an upper bound to avoid overflow.
            long proposed = (long) current * 2L;
            long max = (1L << 30); // about 1 billion entries max
            if (proposed > max) {
                if (current == max) {
                    throw new IllegalStateException("Queue too large.");
                }
                return (int) max;
            }
            return (int) proposed;
        }
    }

    public static void main(String[] args) {
        // 5 Test Cases
        IntQueue q = new IntQueue();

        // Test 1: Enqueue and Peek
        q.enqueue(1);
        q.enqueue(2);
        q.enqueue(3);
        System.out.println("Test1 Peek: " + (q.peek().isPresent() ? q.peek().getAsInt() : null)); // expect 1
        System.out.println("Test1 Dequeue: " + (q.dequeue().isPresent() ? q.dequeue().getAsInt() : null)); // expect 1

        // Test 2: Dequeue remaining and attempt extra dequeue
        System.out.println("Test2 Dequeue: " + (q.dequeue().isPresent() ? q.dequeue().getAsInt() : null)); // expect 2
        System.out.println("Test2 Dequeue: " + (q.dequeue().isPresent() ? q.dequeue().getAsInt() : null)); // expect 3
        System.out.println("Test2 Dequeue Empty: " + (q.dequeue().isPresent() ? q.dequeue().getAsInt() : null)); // expect null

        // Test 3: Interleaved operations
        q.enqueue(10);
        System.out.println("Test3 Peek: " + (q.peek().isPresent() ? q.peek().getAsInt() : null)); // expect 10
        q.enqueue(20);
        System.out.println("Test3 Dequeue: " + (q.dequeue().isPresent() ? q.dequeue().getAsInt() : null)); // expect 10
        System.out.println("Test3 Peek: " + (q.peek().isPresent() ? q.peek().getAsInt() : null)); // expect 20

        // Test 4: Stress growth
        for (int i = 0; i < 100; i++) q.enqueue(i);
        System.out.println("Test4 Size after 100 enqueues: " + q.size()); // expect 101 or more (depending on prior)
        System.out.println("Test4 Peek: " + (q.peek().isPresent() ? q.peek().getAsInt() : null)); // expect 20

        // Consume a few
        for (int i = 0; i < 5; i++) {
            System.out.println("Test4 Dequeue: " + (q.dequeue().isPresent() ? q.dequeue().getAsInt() : null));
        }

        // Test 5: New queue, empty checks
        IntQueue q2 = new IntQueue();
        System.out.println("Test5 IsEmpty: " + q2.isEmpty()); // expect true
        System.out.println("Test5 Peek Empty: " + (q2.peek().isPresent() ? q2.peek().getAsInt() : null)); // expect null
        q2.enqueue(99);
        System.out.println("Test5 Dequeue: " + (q2.dequeue().isPresent() ? q2.dequeue().getAsInt() : null)); // expect 99
        System.out.println("Test5 Dequeue Empty: " + (q2.dequeue().isPresent() ? q2.dequeue().getAsInt() : null)); // expect null
    }
}