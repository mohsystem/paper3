
// INSECURE - For educational purposes only
// This demonstrates poor security practices
public class Task124 {
    // WARNING: Never store sensitive data like this in production!
    private static class SensitiveData {
        String creditCard;
        String ssn;
        String name;
        
        public SensitiveData(String cc, String ssn, String name) {
            this.creditCard = cc;
            this.ssn = ssn;
            this.name = name;
        }
        
        @Override
        public String toString() {
            return "Name: " + name + ", CC: " + creditCard + ", SSN: " + ssn;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("WARNING: This is an INSECURE example for educational purposes only!");
        System.out.println("Never store sensitive data in plain text in production systems.\\n");
        
        // Test cases showing insecure storage
        SensitiveData data1 = new SensitiveData("4532-1234-5678-9010", "123-45-6789", "John Doe");
        SensitiveData data2 = new SensitiveData("5425-2334-3010-9876", "987-65-4321", "Jane Smith");
        SensitiveData data3 = new SensitiveData("3782-822463-10005", "555-55-5555", "Bob Johnson");
        SensitiveData data4 = new SensitiveData("6011-1111-1111-1117", "222-22-2222", "Alice Williams");
        SensitiveData data5 = new SensitiveData("3056-9309-0259-04", "333-33-3333", "Charlie Brown");
        
        System.out.println("Insecurely stored data:");
        System.out.println(data1);
        System.out.println(data2);
        System.out.println(data3);
        System.out.println(data4);
        System.out.println(data5);
    }
}
