
public class Task157 {
    private static class MemoryResource {
        private byte[] data;
        private boolean allocated;
        
        public MemoryResource(int size) {
            if (size <= 0) {
                throw new IllegalArgumentException("Size must be positive");
            }
            this.data = new byte[size];
            this.allocated = true;
            System.out.println("Memory allocated: " + size + " bytes");
        }
        
        public void deallocate() {
            if (allocated) {
                data = null;
                allocated = false;
                System.out.println("Memory deallocated");
                System.gc(); // Suggest garbage collection
            }
        }
        
        public boolean isAllocated() {
            return allocated;
        }
        
        public int getSize() {
            return allocated && data != null ? data.length : 0;
        }
    }
    
    public static MemoryResource allocateMemory(int size) {
        return new MemoryResource(size);
    }
    
    public static void deallocateMemory(MemoryResource resource) {
        if (resource != null) {
            resource.deallocate();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Allocate and deallocate 1024 bytes");
        MemoryResource res1 = allocateMemory(1024);
        System.out.println("Is allocated: " + res1.isAllocated());
        deallocateMemory(res1);
        System.out.println("Is allocated after deallocation: " + res1.isAllocated());
        System.out.println();
        
        System.out.println("Test Case 2: Allocate and deallocate 2048 bytes");
        MemoryResource res2 = allocateMemory(2048);
        System.out.println("Size: " + res2.getSize());
        deallocateMemory(res2);
        System.out.println();
        
        System.out.println("Test Case 3: Multiple allocations");
        MemoryResource res3 = allocateMemory(512);
        MemoryResource res4 = allocateMemory(256);
        deallocateMemory(res3);
        deallocateMemory(res4);
        System.out.println();
        
        System.out.println("Test Case 4: Deallocate null resource");
        deallocateMemory(null);
        System.out.println("No error on null deallocation");
        System.out.println();
        
        System.out.println("Test Case 5: Large allocation");
        MemoryResource res5 = allocateMemory(1048576); // 1MB
        System.out.println("Large allocation size: " + res5.getSize());
        deallocateMemory(res5);
    }
}
