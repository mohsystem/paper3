
class Lse73 {
    static class Person {
        String name;
        int age;
        double height;
        
        public Person(String name, int age, double height) {
            this.name = name;
            this.age = age;
            this.height = height;
        }
    }
    
    public static Person allocatePerson() {
        return new Person("", 0, 0.0);
    }
    
    public static void main(String[] args) {
        // Test case 1
        Person p1 = allocatePerson();
        p1.name = "John";
        p1.age = 25;
        p1.height = 5.9;
        System.out.println("Test 1: " + p1.name + ", " + p1.age + ", " + p1.height);
        
        // Test case 2
        Person p2 = allocatePerson();
        p2.name = "Alice";
        p2.age = 30;
        p2.height = 5.5;
        System.out.println("Test 2: " + p2.name + ", " + p2.age + ", " + p2.height);
        
        // Test case 3
        Person p3 = allocatePerson();
        p3.name = "Bob";
        p3.age = 35;
        p3.height = 6.1;
        System.out.println("Test 3: " + p3.name + ", " + p3.age + ", " + p3.height);
        
        // Test case 4
        Person p4 = allocatePerson();
        p4.name = "Carol";
        p4.age = 28;
        p4.height = 5.7;
        System.out.println("Test 4: " + p4.name + ", " + p4.age + ", " + p4.height);
        
        // Test case 5
        Person p5 = allocatePerson();
        p5.name = "David";
        p5.age = 40;
        p5.height = 5.8;
        System.out.println("Test 5: " + p5.name + ", " + p5.age + ", " + p5.height);
    }
}
