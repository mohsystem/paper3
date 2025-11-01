
class Lse77 {
    static class Person {
        int status;
        String name;
        
        public Person() {
            this.status = 0;
            this.name = "";
        }
    }
    
    public static Person createPerson(String name) {
        Person person = new Person();
        person.status = 0;
        person.name = name;
        return person;
    }
    
    public static void main(String[] args) {
        // Test case 1
        Person person1 = createPerson("John");
        System.out.println("Test 1 - Name: " + person1.name + ", Status: " + person1.status);
        
        // Test case 2
        Person person2 = createPerson("Alice");
        System.out.println("Test 2 - Name: " + person2.name + ", Status: " + person2.status);
        
        // Test case 3
        Person person3 = createPerson("Bob");
        System.out.println("Test 3 - Name: " + person3.name + ", Status: " + person3.status);
        
        // Test case 4
        Person person4 = createPerson("Emma");
        System.out.println("Test 4 - Name: " + person4.name + ", Status: " + person4.status);
        
        // Test case 5
        Person person5 = createPerson("David");
        System.out.println("Test 5 - Name: " + person5.name + ", Status: " + person5.status);
    }
}
