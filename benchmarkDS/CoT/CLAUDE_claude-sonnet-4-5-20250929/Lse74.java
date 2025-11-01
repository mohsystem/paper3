
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
        try {
            Person person = new Person();
            person.status = 0;
            return person;
        } catch (OutOfMemoryError e) {
            System.err.println("Memory allocation failed: " + e.getMessage());
            return null;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Basic allocation");
        Person p1 = allocatePerson();
        if (p1 != null) {
            System.out.println("Person allocated, status: " + p1.status);
        }
        
        System.out.println("\\nTest Case 2: Multiple allocations");
        Person p2 = allocatePerson();
        Person p3 = allocatePerson();
        if (p2 != null && p3 != null) {
            System.out.println("Multiple persons allocated successfully");
        }
        
        System.out.println("\\nTest Case 3: Verify status initialization");
        Person p4 = allocatePerson();
        if (p4 != null) {
            System.out.println("Status is zero: " + (p4.status == 0));
        }
        
        System.out.println("\\nTest Case 4: Modify status");
        Person p5 = allocatePerson();
        if (p5 != null) {
            p5.status = 1;
            System.out.println("Modified status: " + p5.status);
        }
        
        System.out.println("\\nTest Case 5: Null check");
        Person p6 = allocatePerson();
        System.out.println("Allocation result: " + (p6 != null ? "Success" : "Failed"));
    }
}
