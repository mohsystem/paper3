
public class Lse77 {
    static class Person {
        private int status;
        private String name;
        
        public Person() {
            this.status = 0;
            this.name = "";
        }
        
        public int getStatus() {
            return status;
        }
        
        public void setStatus(int status) {
            this.status = status;
        }
        
        public String getName() {
            return name;
        }
        
        public void setName(String name) {
            if (name != null) {
                this.name = name;
            }
        }
        
        @Override
        public String toString() {
            return "Person{status=" + status + ", name='" + name + "'}";
        }
    }
    
    public static Person createPerson(String name) {
        Person person = new Person();
        person.setStatus(0);
        person.setName(name);
        return person;
    }
    
    public static void main(String[] args) {
        // Test case 1: Create person with name "John"
        Person person1 = createPerson("John");
        System.out.println("Test 1: " + person1);
        
        // Test case 2: Create person with name "Alice"
        Person person2 = createPerson("Alice");
        System.out.println("Test 2: " + person2);
        
        // Test case 3: Create person with name "Bob"
        Person person3 = createPerson("Bob");
        System.out.println("Test 3: " + person3);
        
        // Test case 4: Create person with empty name
        Person person4 = createPerson("");
        System.out.println("Test 4: " + person4);
        
        // Test case 5: Create person with null name (secure handling)
        Person person5 = createPerson(null);
        System.out.println("Test 5: " + person5);
    }
}
