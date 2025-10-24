import java.util.*;

public class Task148 {
    static class IntStack {
        private int[] data;
        private int size;

        public IntStack(int initialCapacity) {
            if (initialCapacity <= 0) initialCapacity = 8;
            data = new int[initialCapacity];
            size = 0;
        }

        private void ensureCapacity() {
            if (size == data.length) {
                data = Arrays.copyOf(data, data.length * 2);
            }
        }

        public void push(int value) {
            ensureCapacity();
            data[size++] = value;
        }

        public int pop() {
            if (size == 0) throw new IllegalStateException("Stack underflow");
            return data[--size];
        }

        public int peek() {
            if (size == 0) throw new IllegalStateException("Stack is empty");
            return data[size - 1];
        }

        public boolean isEmpty() {
            return size == 0;
        }

        public int size() {
            return size;
        }
    }

    public static void main(String[] args) {
        IntStack s = new IntStack(4);
        s.push(10);
        s.push(20);
        s.push(30);
        System.out.println(s.peek()); // Test 1
        System.out.println(s.pop());  // Test 2
        s.push(40);
        System.out.println(s.peek()); // Test 3
        System.out.println(s.pop());  // Test 4
        System.out.println(s.pop());  // Test 5
    }
}