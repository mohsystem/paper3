
public class Task115 {
    // Java doesn't support bit fields directly, so we use bitwise operations
    private int data;
    
    // Bit field positions and masks
    private static final int FLAG1_MASK = 0x01;      // 1 bit
    private static final int FLAG2_MASK = 0x02;      // 1 bit
    private static final int VALUE_MASK = 0x3C;      // 4 bits (positions 2-5)
    private static final int VALUE_SHIFT = 2;
    private static final int STATUS_MASK = 0xC0;     // 2 bits (positions 6-7)
    private static final int STATUS_SHIFT = 6;
    
    public Task115() {
        this.data = 0;
    }
    
    public void setFlag1(boolean value) {
        if (value) {
            data |= FLAG1_MASK;
        } else {
            data &= ~FLAG1_MASK;
        }
    }
    
    public boolean getFlag1() {
        return (data & FLAG1_MASK) != 0;
    }
    
    public void setFlag2(boolean value) {
        if (value) {
            data |= FLAG2_MASK;
        } else {
            data &= ~FLAG2_MASK;
        }
    }
    
    public boolean getFlag2() {
        return (data & FLAG2_MASK) != 0;
    }
    
    public void setValue(int value) {
        if (value < 0 || value > 15) {
            throw new IllegalArgumentException("Value must be between 0 and 15");
        }
        data = (data & ~VALUE_MASK) | ((value << VALUE_SHIFT) & VALUE_MASK);
    }
    
    public int getValue() {
        return (data & VALUE_MASK) >> VALUE_SHIFT;
    }
    
    public void setStatus(int status) {
        if (status < 0 || status > 3) {
            throw new IllegalArgumentException("Status must be between 0 and 3");
        }
        data = (data & ~STATUS_MASK) | ((status << STATUS_SHIFT) & STATUS_MASK);
    }
    
    public int getStatus() {
        return (data & STATUS_MASK) >> STATUS_SHIFT;
    }
    
    @Override
    public String toString() {
        return String.format("BitField{flag1=%b, flag2=%b, value=%d, status=%d}", 
                           getFlag1(), getFlag2(), getValue(), getStatus());
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Initialize and check default values");
        Task115 bf1 = new Task115();
        System.out.println(bf1);
        
        System.out.println("\\nTest Case 2: Set all flags and values");
        Task115 bf2 = new Task115();
        bf2.setFlag1(true);
        bf2.setFlag2(true);
        bf2.setValue(10);
        bf2.setStatus(2);
        System.out.println(bf2);
        
        System.out.println("\\nTest Case 3: Set partial values");
        Task115 bf3 = new Task115();
        bf3.setFlag1(true);
        bf3.setValue(15);
        System.out.println(bf3);
        
        System.out.println("\\nTest Case 4: Toggle flags");
        Task115 bf4 = new Task115();
        bf4.setFlag1(true);
        bf4.setFlag2(false);
        bf4.setValue(7);
        bf4.setStatus(3);
        System.out.println(bf4);
        bf4.setFlag1(false);
        bf4.setFlag2(true);
        System.out.println(bf4);
        
        System.out.println("\\nTest Case 5: Maximum values");
        Task115 bf5 = new Task115();
        bf5.setFlag1(true);
        bf5.setFlag2(true);
        bf5.setValue(15);
        bf5.setStatus(3);
        System.out.println(bf5);
    }
}
