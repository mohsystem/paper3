
class Lse75 {
    static class Person {
        int status;
        String name;
        int age;
        
        public Person() {
            this.status = 0;
            this.name = "";
            this.age = 0;
        }
    }
    
    public static Person allocatePerson() {
        Person person = new Person();
        person.status = 0;
        return person;
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic allocation
        Person person1 = allocatePerson();
        System.out.println("Test 1 - Status: " + person1.status);
        
        // Test case 2: Multiple allocations
        Person person2 = allocatePerson();
        System.out.println("Test 2 - Status: " + person2.status);
        
        // Test case 3: Verify independence
        person1.status = 1;
        Person person3 = allocatePerson();
        System.out.println("Test 3 - Status: " + person3.status);
        
        // Test case 4: Check not null
        Person person4 = allocatePerson();
        System.out.println("Test 4 - Not null: " + (person4 != null));
        
        // Test case 5: Verify initial values
        Person person5 = allocatePerson();
        System.out.println("Test 5 - Status: " + person5.status + ", Age: " + person5.age);
    }
}
