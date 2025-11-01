
import java.util.ArrayList;
import java.util.List;

public class Lse74 {
    // Person class representing a person with a status field
    static class Person {
        private int status;
        
        // Constructor initializes status to 0
        public Person() {
            this.status = 0;
        }
        
        public int getStatus() {
            return status;
        }
        
        public void setStatus(int status) {
            this.status = status;
        }
        
        @Override
        public String toString() {
            return "Person{status=" + status + "}";
        }
    }
    
    // Memory allocation method that creates a new Person object
    // Returns a reference to the newly allocated Person
    public static Person allocatePerson() {
        // Allocate memory for a new Person object
        // Java handles memory allocation automatically via 'new'
        // The Person constructor sets status to 0 by default
        Person person = new Person();
        
        // Explicitly set status to 0 (redundant but matches specification)
        person.setStatus(0);
        
        return person;
    }
    
    public static void main(String[] args) {
        System.out.println("Testing Person allocation:");
        
        // Test case 1: Allocate a single person
        Person person1 = allocatePerson();
        System.out.println("Test 1 - " + person1);
        
        // Test case 2: Allocate another person
        Person person2 = allocatePerson();
        System.out.println("Test 2 - " + person2);
        
        // Test case 3: Verify status is 0
        Person person3 = allocatePerson();
        System.out.println("Test 3 - Status is: " + person3.getStatus());
        
        // Test case 4: Allocate multiple persons
        List<Person> people = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            people.add(allocatePerson());
        }
        System.out.println("Test 4 - Allocated " + people.size() + " persons");
        
        // Test case 5: Verify all have status 0
        Person person4 = allocatePerson();
        Person person5 = allocatePerson();
        boolean allZero = (person4.getStatus() == 0) && (person5.getStatus() == 0);
        System.out.println("Test 5 - All status fields are 0: " + allZero);
    }
}
