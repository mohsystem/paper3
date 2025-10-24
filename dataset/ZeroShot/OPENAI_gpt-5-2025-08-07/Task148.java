import java.util.Arrays;
import java.util.NoSuchElementException;

public class Task148 {
    static class IntStack {
        private int[] data;
        private int size;
        private static final int DEFAULT_CAPACITY = 16;
        private static final int MAX_CAPACITY = Integer.MAX_VALUE - 8;

        IntStack() {
            this.data = new int[DEFAULT_CAPACITY];
            this.size = 0;
        }

        public void push(int value) {
            ensureCapacity(size + 1);
            data[size++] = value;
        }

        public int pop() {
            if (size == 0) {
                throw new NoSuchElementException("Stack underflow");
            }
            int val = data[--size];
            data[size] = 0; // clear reference
            return val;
        }

        public int peek() {
            if (size == 0) {
                throw new NoSuchElementException("Stack underflow");
            }
            return data[size - 1];
        }

        public int size() {
            return size;
        }

        private void ensureCapacity(int minCapacity) {
            if (minCapacity <= data.length) return;
            int newCap = data.length == 0 ? DEFAULT_CAPACITY : data.length;
            while (newCap < minCapacity) {
                long doubled = (long) newCap * 2L;
                if (doubled > MAX_CAPACITY) {
                    newCap = MAX_CAPACITY;
                    break;
                }
                newCap = (int) doubled;
            }
            if (newCap < minCapacity) {
                throw new OutOfMemoryError("Required stack capacity too large");
            }
            data = Arrays.copyOf(data, newCap);
        }
    }

    public static void main(String[] args) {
        IntStack s = new IntStack();
        try {
            s.push(1); s.push(2); s.push(3);
            System.out.println("Test1 Peek: " + s.peek());
        } catch (Exception e) {
            System.out.println("Test1 Exception: " + e.getMessage());
        }
        try {
            System.out.println("Test2 Pop: " + s.pop());
        } catch (Exception e) {
            System.out.println("Test2 Exception: " + e.getMessage());
        }
        try {
            System.out.println("Test3 Pop: " + s.pop());
        } catch (Exception e) {
            System.out.println("Test3 Exception: " + e.getMessage());
        }
        try {
            s.push(4);
            System.out.println("Test4 Pop: " + s.pop());
        } catch (Exception e) {
            System.out.println("Test4 Exception: " + e.getMessage());
        }
        try {
            System.out.println("Test5 Pop: " + s.pop()); // should pop 1
            System.out.println("Test5 Pop: " + s.pop()); // underflow
        } catch (Exception e) {
            System.out.println("Test5 Exception: " + e.getMessage());
        }
    }
}