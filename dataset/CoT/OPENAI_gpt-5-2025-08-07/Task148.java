import java.util.OptionalInt;

public class Task148 {
    // Simple, resizable, safe integer stack
    static class SafeStack {
        private int[] arr;
        private int size;

        public SafeStack() {
            this.arr = new int[16];
            this.size = 0;
        }

        // Push returns true on success, false if allocation fails
        public boolean push(int value) {
            if (size == arr.length) {
                int newCap = arr.length <= (Integer.MAX_VALUE / 2) ? arr.length * 2 : (Integer.MAX_VALUE - 8);
                if (newCap <= arr.length) {
                    return false;
                }
                try {
                    int[] newArr = new int[newCap];
                    System.arraycopy(arr, 0, newArr, 0, size);
                    arr = newArr;
                } catch (OutOfMemoryError e) {
                    return false;
                }
            }
            arr[size++] = value;
            return true;
        }

        // Pop returns OptionalInt.empty() when stack is empty
        public OptionalInt pop() {
            if (size == 0) return OptionalInt.empty();
            int v = arr[--size];
            arr[size] = 0; // clear
            return OptionalInt.of(v);
        }

        // Peek returns OptionalInt.empty() when stack is empty
        public OptionalInt peek() {
            if (size == 0) return OptionalInt.empty();
            return OptionalInt.of(arr[size - 1]);
        }

        public boolean isEmpty() {
            return size == 0;
        }

        public int size() {
            return size;
        }
    }

    // 5 test cases
    public static void main(String[] args) {
        SafeStack st = new SafeStack();

        // Test 1: Push 1,2,3 then peek
        boolean t1a = st.push(1);
        boolean t1b = st.push(2);
        boolean t1c = st.push(3);
        OptionalInt t1peek = st.peek();
        System.out.println("Test1: pushes=" + t1a + "," + t1b + "," + t1c + " peekOk=" + t1peek.isPresent() + " peekVal=" + (t1peek.isPresent() ? t1peek.getAsInt() : null));

        // Test 2: Pop once
        OptionalInt t2 = st.pop();
        System.out.println("Test2: popOk=" + t2.isPresent() + " val=" + (t2.isPresent() ? t2.getAsInt() : null));

        // Test 3: Peek after pop
        OptionalInt t3 = st.peek();
        System.out.println("Test3: peekOk=" + t3.isPresent() + " val=" + (t3.isPresent() ? t3.getAsInt() : null));

        // Test 4: Pop remaining and attempt extra pop
        OptionalInt t4a = st.pop();
        OptionalInt t4b = st.pop();
        OptionalInt t4c = st.pop(); // should be empty
        System.out.println("Test4: popVals=" + (t4a.isPresent() ? t4a.getAsInt() : null) + "," + (t4b.isPresent() ? t4b.getAsInt() : null) + " extraPopOk=" + t4c.isPresent());

        // Test 5: Push after empty and peek
        boolean t5a = st.push(42);
        OptionalInt t5b = st.peek();
        System.out.println("Test5: pushOk=" + t5a + " peekOk=" + t5b.isPresent() + " val=" + (t5b.isPresent() ? t5b.getAsInt() : null));
    }
}