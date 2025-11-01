public class Lse74 {
    // Step 1: Define Person with a status field.
    static class Person {
        int status;

        Person() {
            // Step 3: Secure initialization.
            this.status = 0;
        }
    }

    // Step 3: Secure allocation method to create a new Person and set status to 0.
    public static Person allocatePerson() {
        Person p = new Person();
        // Explicitly ensure status is 0.
        p.status = 0;
        return p;
    }

    // Step 5: Main with 5 test cases.
    public static void main(String[] args) {
        Person p1 = allocatePerson();
        Person p2 = allocatePerson();
        Person p3 = allocatePerson();
        Person p4 = allocatePerson();
        Person p5 = allocatePerson();

        System.out.println("Test1 status: " + p1.status);
        System.out.println("Test2 status: " + p2.status);
        System.out.println("Test3 status: " + p3.status);
        System.out.println("Test4 status: " + p4.status);
        System.out.println("Test5 status: " + p5.status);
    }
}