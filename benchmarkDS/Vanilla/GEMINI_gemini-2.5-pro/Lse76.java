class Person {
    int status;
    // Adding another field for a more realistic structure
    int age;
}

public class Lse76 {

    /**
     * Allocates memory for a new Person object, sets its status to 0,
     * and returns a reference to it.
     * @return A new Person object with status initialized to 0.
     */
    public static Person allocatePerson() {
        // In Java, 'new' allocates memory for an object.
        Person newPerson = new Person();
        
        // Set the status to 0 as per the requirement.
        newPerson.status = 0;
        
        return newPerson;
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");
        for (int i = 1; i <= 5; i++) {
            Person p = allocatePerson();
            // In Java, allocation failure throws an OutOfMemoryError, so we don't check for null here.
            // The program would crash before this point if allocation failed.
            System.out.println("Test Case " + i + ": Allocated Person object. Status: " + p.status);
            // Java's garbage collector will automatically deallocate the memory when the object is no longer referenced.
        }
    }
}