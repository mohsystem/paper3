
class Lse74 {
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
        Person p = new Person();
        p.status = 0;
        return p;
    }
    
    public static void main(String[] args) {
        // Test case 1: Allocate a person and verify status is 0
        Person person1 = allocatePerson();
        System.out.println("Test 1 - Status: " + person1.status);
        
        // Test case 2: Allocate another person and verify status is 0
        Person person2 = allocatePerson();
        System.out.println("Test 2 - Status: " + person2.status);
        
        // Test case 3: Modify status and allocate new person
        person1.status = 5;
        Person person3 = allocatePerson();
        System.out.println("Test 3 - Person1 Status: " + person1.status + ", Person3 Status: " + person3.status);
        
        // Test case 4: Verify multiple allocations
        Person person4 = allocatePerson();
        Person person5 = allocatePerson();
        System.out.println("Test 4 - Person4 Status: " + person4.status + ", Person5 Status: " + person5.status);
        
        // Test case 5: Verify objects are different
        System.out.println("Test 5 - Are person1 and person2 different objects? " + (person1 != person2));
    }
}
