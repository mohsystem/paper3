
public class Lse77 {
    static class Person {
        private int status;
        private String name;
        
        public Person(int status, String name) {
            this.status = status;
            this.name = name != null ? name : "";
        }
        
        public int getStatus() {
            return status;
        }
        
        public String getName() {
            return name;
        }
        
        @Override
        public String toString() {
            return "Person{status=" + status + ", name='" + name + "'}";
        }
    }
    
    public static Person createPerson() {
        Person person = new Person(0, "John");
        return person;
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic creation
        Person person1 = createPerson();
        System.out.println("Test 1: " + person1);
        
        // Test case 2: Verify status
        Person person2 = createPerson();
        System.out.println("Test 2 - Status: " + person2.getStatus());
        
        // Test case 3: Verify name
        Person person3 = createPerson();
        System.out.println("Test 3 - Name: " + person3.getName());
        
        // Test case 4: Multiple instances
        Person person4 = createPerson();
        Person person5 = createPerson();
        System.out.println("Test 4 - Multiple: " + person4 + ", " + person5);
        
        // Test case 5: Object independence
        Person person6 = createPerson();
        System.out.println("Test 5 - Independent: " + person6);
    }
}
