import java.nio.ByteBuffer;
import java.util.Arrays;

public class Lse77 {
    public static ByteBuffer alloc(int size) {
        if (size < 0) throw new IllegalArgumentException("size must be non-negative");
        return ByteBuffer.allocate(size);
    }

    public static Person createPerson(String name, int status) {
        return new Person(status, name);
    }

    public static class Person {
        public int status;
        public String name;

        public Person(int status, String name) {
            this.status = status;
            this.name = name;
        }

        @Override
        public String toString() {
            return "Person{name='" + name + "', status=" + status + "}";
        }
    }

    public static void main(String[] args) {
        int[] sizes = {8, 16, 32, 64, 128};
        for (int s : sizes) {
            ByteBuffer buf = alloc(s);
            System.out.println("Allocated ByteBuffer of capacity: " + buf.capacity());
        }

        Person john = createPerson("John", 0);
        System.out.println("Created person: " + john);

        // Additional small validations
        ByteBuffer b = alloc(10);
        Arrays.fill(b.array(), (byte) 1);
        System.out.println("Sample buffer first 3 bytes: " + b.get(0) + "," + b.get(1) + "," + b.get(2));
    }
}