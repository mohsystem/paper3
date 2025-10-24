
import java.util.ArrayList;
import java.util.List;

public class Task157 {
    // Memory resource class to track allocations
    private static class MemoryResource {
        private final int size;
        private final byte[] data;
        private boolean allocated;
        
        public MemoryResource(int size) {
            if (size <= 0) {
                throw new IllegalArgumentException("Size must be positive");
            }
            this.size = size;
            this.data = new byte[size];
            this.allocated = true;
        }
        
        public void deallocate() {
            if (!allocated) {
                throw new IllegalStateException("Resource already deallocated");
            }
            // Clear sensitive data before deallocation
            for (int i = 0; i < data.length; i++) {
                data[i] = 0;
            }
            allocated = false;
        }
        
        public boolean isAllocated() {
            return allocated;
        }
        
        public int getSize() {
            return size;
        }
    }
    
    // Safe memory manager to prevent memory leaks
    public static MemoryResource allocateMemory(int size) {
        if (size <= 0 || size > Integer.MAX_VALUE / 2) {
            throw new IllegalArgumentException("Invalid memory size");
        }
        return new MemoryResource(size);
    }
    
    public static void deallocateMemory(MemoryResource resource) {
        if (resource == null) {
            throw new IllegalArgumentException("Resource cannot be null");
        }
        resource.deallocate();
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Basic allocation and deallocation");
        try {
            MemoryResource res1 = allocateMemory(1024);
            System.out.println("Allocated: " + res1.getSize() + " bytes, Status: " + res1.isAllocated());
            deallocateMemory(res1);
            System.out.println("Deallocated, Status: " + res1.isAllocated());
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 2: Multiple allocations");
        try {
            List<MemoryResource> resources = new ArrayList<>();
            for (int i = 1; i <= 3; i++) {
                MemoryResource res = allocateMemory(512 * i);
                resources.add(res);
                System.out.println("Allocated resource " + i + ": " + res.getSize() + " bytes");
            }
            for (MemoryResource res : resources) {
                deallocateMemory(res);
            }
            System.out.println("All resources deallocated");
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 3: Invalid size (negative)");
        try {
            MemoryResource res3 = allocateMemory(-100);
        } catch (Exception e) {
            System.out.println("Caught expected error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 4: Double deallocation prevention");
        try {
            MemoryResource res4 = allocateMemory(2048);
            deallocateMemory(res4);
            deallocateMemory(res4);
        } catch (Exception e) {
            System.out.println("Caught expected error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 5: Null resource deallocation");
        try {
            deallocateMemory(null);
        } catch (Exception e) {
            System.out.println("Caught expected error: " + e.getMessage());
        }
    }
}
