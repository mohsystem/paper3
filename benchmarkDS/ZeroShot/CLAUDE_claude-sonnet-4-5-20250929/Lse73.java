
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
        
        @Override
        public String toString() {
            return "Person{name='" + name + "', age=" + age + ", height=" + height + "}";
        }
    }
    
    public static Person allocatePerson(String name, int age, double height) {
        return new Person(name, age, height);
    }
    
    public static void main(String[] args) {
        // Test case 1
        Person p1 = allocatePerson("John Doe", 30, 5.9);
        System.out.println("Test 1: " + p1);
        
        // Test case 2
        Person p2 = allocatePerson("Jane Smith", 25, 5.5);
        System.out.println("Test 2: " + p2);
        
        // Test case 3
        Person p3 = allocatePerson("Bob Johnson", 45, 6.1);
        System.out.println("Test 3: " + p3);
        
        // Test case 4
        Person p4 = allocatePerson("Alice Williams", 35, 5.7);
        System.out.println("Test 4: " + p4);
        
        // Test case 5
        Person p5 = allocatePerson("Charlie Brown", 28, 5.8);
        System.out.println("Test 5: " + p5);
    }
}
