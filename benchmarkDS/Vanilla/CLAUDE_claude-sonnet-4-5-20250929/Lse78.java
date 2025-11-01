
class Lse78 {
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
        // Test case 1
        Person p1 = allocatePerson();
        System.out.println("Test 1 - Status: " + p1.status);
        
        // Test case 2
        Person p2 = allocatePerson();
        System.out.println("Test 2 - Status: " + p2.status);
        
        // Test case 3
        Person p3 = allocatePerson();
        System.out.println("Test 3 - Status: " + p3.status);
        
        // Test case 4
        Person p4 = allocatePerson();
        System.out.println("Test 4 - Status: " + p4.status);
        
        // Test case 5
        Person p5 = allocatePerson();
        System.out.println("Test 5 - Status: " + p5.status);
    }
}
