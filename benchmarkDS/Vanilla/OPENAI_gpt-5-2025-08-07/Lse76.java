public class Lse76 {
    static class Person {
        int status;
    }

    public static Person allocatePerson() {
        Person p = new Person();
        p.status = 0;
        return p;
    }

    public static void main(String[] args) {
        // 5 test cases
        for (int i = 0; i < 5; i++) {
            Person p = allocatePerson();
            System.out.println(p.status);
        }
    }
}