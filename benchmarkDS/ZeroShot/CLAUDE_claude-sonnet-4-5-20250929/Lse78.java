
public class Lse78 {
    static class Person {
        String name;
        int age;
        int status;
        
        public Person() {
            this.status = 0;
        }
        
        public Person(String name, int age) {
            this.name = name;
            this.age = age;
            this.status = 0;
        }
        
        @Override
        public String toString() {
            return "Person{name='" + name + "', age=" + age + ", status=" + status + "}";
        }
    }
    
    public static Person allocatePerson(String name, int age) {
        Person person = new Person(name, age);
        return person;
    }
    
    public static void main(String[] args) {
        // Test case 1
        Person p1 = allocatePerson("Alice", 25);
        System.out.println("Test 1: " + p1);
        
        // Test case 2
        Person p2 = allocatePerson("Bob", 30);
        System.out.println("Test 2: " + p2);
        
        // Test case 3
        Person p3 = allocatePerson("Charlie", 35);
        System.out.println("Test 3: " + p3);
        
        // Test case 4
        Person p4 = allocatePerson("David", 40);
        System.out.println("Test 4: " + p4);
        
        // Test case 5
        Person p5 = allocatePerson("Eve", 28);
        System.out.println("Test 5: " + p5);
    }
}
