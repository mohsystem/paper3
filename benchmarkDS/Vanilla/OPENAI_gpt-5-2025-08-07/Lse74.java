import java.util.*;

class Person {
    int status;
}

public class Lse74 {
    // Allocates a new Person and sets status to 0
    public static Person allocatePerson() {
        Person p = new Person();
        p.status = 0;
        return p;
    }

    public static void main(String[] args) {
        // 5 test cases
        Person p1 = allocatePerson();
        Person p2 = allocatePerson();
        Person p3 = allocatePerson();
        Person p4 = allocatePerson();
        Person p5 = allocatePerson();

        // Print initial statuses
        System.out.println("p1.status = " + p1.status);
        System.out.println("p2.status = " + p2.status);
        System.out.println("p3.status = " + p3.status);
        System.out.println("p4.status = " + p4.status);
        System.out.println("p5.status = " + p5.status);
    }
}