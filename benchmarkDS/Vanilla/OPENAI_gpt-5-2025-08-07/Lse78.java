import java.util.Arrays;

public class Lse78 {
    public static byte[] allocateMemory(int size) {
        if (size < 0) throw new IllegalArgumentException("size must be non-negative");
        return new byte[size];
    }

    public static Person allocatePerson() {
        Person p = new Person();
        p.status = 0;
        return p;
    }

    public static class Person {
        public int status;
    }

    public static void main(String[] args) {
        int[] sizes = {0, 1, 16, 1024, 65536};

        // 5 test cases for memory allocation
        for (int s : sizes) {
            byte[] mem = allocateMemory(s);
            System.out.println("Allocated memory size: " + mem.length);
        }

        // 5 test cases for person allocation
        for (int i = 0; i < 5; i++) {
            Person p = allocatePerson();
            System.out.println("Person status: " + p.status);
        }
    }
}