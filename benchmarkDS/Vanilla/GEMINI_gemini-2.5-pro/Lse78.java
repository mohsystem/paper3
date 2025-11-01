class Person {
    int status;
    // You can add other person attributes here
    // String name;
    // int age;
}

public class Lse78 {

    /**
     * Allocates a new Person object and sets its status to 0.
     * @return A new Person object with status initialized to 0.
     */
    public static Person allocatePerson() {
        Person newPerson = new Person();
        newPerson.status = 0;
        return newPerson;
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1
        Person p1 = allocatePerson();
        System.out.println("Test Case 1: Person allocated with status: " + p1.status);

        // Test Case 2
        Person p2 = allocatePerson();
        System.out.println("Test Case 2: Person allocated with status: " + p2.status);

        // Test Case 3
        Person p3 = allocatePerson();
        System.out.println("Test Case 3: Person allocated with status: " + p3.status);

        // Test Case 4
        Person p4 = allocatePerson();
        System.out.println("Test Case 4: Person allocated with status: " + p4.status);

        // Test Case 5
        Person p5 = allocatePerson();
        System.out.println("Test Case 5: Person allocated with status: " + p5.status);
    }
}