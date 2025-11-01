public class Lse76 {
    public static final class Person {
        private int status;
        private Person() {}
        public int getStatus() { return status; }
        private void setStatus(int s) { this.status = s; }
    }

    public static Person allocatePerson() {
        try {
            Person p = new Person();
            p.setStatus(0);
            return p;
        } catch (OutOfMemoryError e) {
            return null;
        }
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            Person p = allocatePerson();
            if (p == null) {
                System.out.println("Allocation failed");
            } else {
                System.out.println("Test " + i + ": status=" + p.getStatus());
            }
        }
    }
}