
public class Task157 {
    private static class MemoryResource {
        private int[] data;
        private int size;
        
        public MemoryResource(int size) {
            this.size = size;
            this.data = new int[size];
            System.out.println("Allocated memory for " + size + " integers");
        }
        
        public void setData(int index, int value) {
            if (index >= 0 && index < size) {
                data[index] = value;
            }
        }
        
        public int getData(int index) {
            if (index >= 0 && index < size) {
                return data[index];
            }
            return -1;
        }
        
        public void deallocate() {
            data = null;
            System.out.println("Deallocated memory for " + size + " integers");
        }
        
        public int getSize() {
            return size;
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
        System.out.println("Test Case 1: Allocate and deallocate 10 integers");
        MemoryResource res1 = allocateMemory(10);
        res1.setData(0, 100);
        res1.setData(5, 500);
        System.out.println("Data at index 0: " + res1.getData(0));
        System.out.println("Data at index 5: " + res1.getData(5));
        deallocateMemory(res1);
        System.out.println();
        
        System.out.println("Test Case 2: Allocate and deallocate 100 integers");
        MemoryResource res2 = allocateMemory(100);
        for (int i = 0; i < 10; i++) {
            res2.setData(i, i * 10);
        }
        System.out.println("Data at index 7: " + res2.getData(7));
        deallocateMemory(res2);
        System.out.println();
        
        System.out.println("Test Case 3: Allocate and deallocate 1 integer");
        MemoryResource res3 = allocateMemory(1);
        res3.setData(0, 999);
        System.out.println("Data at index 0: " + res3.getData(0));
        deallocateMemory(res3);
        System.out.println();
        
        System.out.println("Test Case 4: Allocate and deallocate 1000 integers");
        MemoryResource res4 = allocateMemory(1000);
        res4.setData(999, 12345);
        System.out.println("Data at index 999: " + res4.getData(999));
        deallocateMemory(res4);
        System.out.println();
        
        System.out.println("Test Case 5: Multiple allocations and deallocations");
        MemoryResource res5a = allocateMemory(50);
        MemoryResource res5b = allocateMemory(75);
        res5a.setData(25, 2500);
        res5b.setData(50, 5000);
        System.out.println("Resource A at index 25: " + res5a.getData(25));
        System.out.println("Resource B at index 50: " + res5b.getData(50));
        deallocateMemory(res5a);
        deallocateMemory(res5b);
    }
}
