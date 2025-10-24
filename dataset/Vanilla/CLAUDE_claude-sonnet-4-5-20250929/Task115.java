
class Task115 {
    // Java doesn't support bit fields directly, so we use bit manipulation
    static class BitFields {
        private int value;
        
        // Field 1: 3 bits (0-2)
        public void setField1(int val) {
            value = (value & ~0x07) | (val & 0x07);
        }
        
        public int getField1() {
            return value & 0x07;
        }
        
        // Field 2: 5 bits (3-7)
        public void setField2(int val) {
            value = (value & ~0xF8) | ((val & 0x1F) << 3);
        }
        
        public int getField2() {
            return (value >> 3) & 0x1F;
        }
        
        // Field 3: 8 bits (8-15)
        public void setField3(int val) {
            value = (value & ~0xFF00) | ((val & 0xFF) << 8);
        }
        
        public int getField3() {
            return (value >> 8) & 0xFF;
        }
        
        public void display() {
            System.out.println("Field1: " + getField1() + ", Field2: " + getField2() + ", Field3: " + getField3());
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        BitFields bf1 = new BitFields();
        bf1.setField1(5);
        bf1.setField2(20);
        bf1.setField3(100);
        System.out.print("Test 1: ");
        bf1.display();
        
        // Test case 2
        BitFields bf2 = new BitFields();
        bf2.setField1(7);
        bf2.setField2(31);
        bf2.setField3(255);
        System.out.print("Test 2: ");
        bf2.display();
        
        // Test case 3
        BitFields bf3 = new BitFields();
        bf3.setField1(0);
        bf3.setField2(0);
        bf3.setField3(0);
        System.out.print("Test 3: ");
        bf3.display();
        
        // Test case 4
        BitFields bf4 = new BitFields();
        bf4.setField1(3);
        bf4.setField2(15);
        bf4.setField3(128);
        System.out.print("Test 4: ");
        bf4.display();
        
        // Test case 5
        BitFields bf5 = new BitFields();
        bf5.setField1(1);
        bf5.setField2(10);
        bf5.setField3(50);
        System.out.print("Test 5: ");
        bf5.display();
    }
}
