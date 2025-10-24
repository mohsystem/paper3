
public class Task115 {
    // Simulating bit fields for a network packet header
    private int version;      // 4 bits (0-15)
    private int priority;     // 3 bits (0-7)
    private int flag;         // 1 bit (0-1)
    private int payloadLength; // 8 bits (0-255)
    
    public Task115() {
        this.version = 0;
        this.priority = 0;
        this.flag = 0;
        this.payloadLength = 0;
    }
    
    // Setters with validation
    public boolean setVersion(int value) {
        if (value < 0 || value > 15) {
            System.out.println("Error: Version must be 0-15");
            return false;
        }
        this.version = value & 0x0F; // Mask to 4 bits
        return true;
    }
    
    public boolean setPriority(int value) {
        if (value < 0 || value > 7) {
            System.out.println("Error: Priority must be 0-7");
            return false;
        }
        this.priority = value & 0x07; // Mask to 3 bits
        return true;
    }
    
    public boolean setFlag(int value) {
        if (value < 0 || value > 1) {
            System.out.println("Error: Flag must be 0 or 1");
            return false;
        }
        this.flag = value & 0x01; // Mask to 1 bit
        return true;
    }
    
    public boolean setPayloadLength(int value) {
        if (value < 0 || value > 255) {
            System.out.println("Error: PayloadLength must be 0-255");
            return false;
        }
        this.payloadLength = value & 0xFF; // Mask to 8 bits
        return true;
    }
    
    // Getters
    public int getVersion() {
        return version;
    }
    
    public int getPriority() {
        return priority;
    }
    
    public int getFlag() {
        return flag;
    }
    
    public int getPayloadLength() {
        return payloadLength;
    }
    
    // Pack all fields into a single 16-bit value
    public int packToInt() {
        return ((version & 0x0F) << 12) | 
               ((priority & 0x07) << 9) | 
               ((flag & 0x01) << 8) | 
               (payloadLength & 0xFF);
    }
    
    // Unpack from 16-bit value
    public void unpackFromInt(int packed) {
        version = (packed >> 12) & 0x0F;
        priority = (packed >> 9) & 0x07;
        flag = (packed >> 8) & 0x01;
        payloadLength = packed & 0xFF;
    }
    
    @Override
    public String toString() {
        return String.format("Version: %d, Priority: %d, Flag: %d, PayloadLength: %d (Packed: 0x%04X)",
                             version, priority, flag, payloadLength, packToInt());
    }
    
    public static void main(String[] args) {
        System.out.println("=== Bit Field Struct Demonstration ===\\n");
        
        // Test Case 1: Normal values
        System.out.println("Test Case 1: Setting normal values");
        Task115 packet1 = new Task115();
        packet1.setVersion(4);
        packet1.setPriority(3);
        packet1.setFlag(1);
        packet1.setPayloadLength(128);
        System.out.println(packet1);
        System.out.println();
        
        // Test Case 2: Maximum values
        System.out.println("Test Case 2: Setting maximum values");
        Task115 packet2 = new Task115();
        packet2.setVersion(15);
        packet2.setPriority(7);
        packet2.setFlag(1);
        packet2.setPayloadLength(255);
        System.out.println(packet2);
        System.out.println();
        
        // Test Case 3: Minimum values
        System.out.println("Test Case 3: Setting minimum values");
        Task115 packet3 = new Task115();
        packet3.setVersion(0);
        packet3.setPriority(0);
        packet3.setFlag(0);
        packet3.setPayloadLength(0);
        System.out.println(packet3);
        System.out.println();
        
        // Test Case 4: Invalid values (should show errors)
        System.out.println("Test Case 4: Attempting invalid values");
        Task115 packet4 = new Task115();
        packet4.setVersion(20);  // Invalid
        packet4.setPriority(10); // Invalid
        packet4.setFlag(2);      // Invalid
        packet4.setPayloadLength(300); // Invalid
        System.out.println(packet4);
        System.out.println();
        
        // Test Case 5: Pack and unpack
        System.out.println("Test Case 5: Pack and unpack operations");
        Task115 packet5 = new Task115();
        packet5.setVersion(7);
        packet5.setPriority(5);
        packet5.setFlag(1);
        packet5.setPayloadLength(200);
        System.out.println("Original: " + packet5);
        int packed = packet5.packToInt();
        Task115 packet6 = new Task115();
        packet6.unpackFromInt(packed);
        System.out.println("Unpacked: " + packet6);
    }
}
