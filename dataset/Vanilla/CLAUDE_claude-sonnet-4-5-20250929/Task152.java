
import java.util.LinkedList;

public class Task152 {
    private static class HashNode {
        String key;
        int value;
        
        public HashNode(String key, int value) {
            this.key = key;
            this.value = value;
        }
    }
    
    private LinkedList<HashNode>[] table;
    private int capacity;
    private int size;
    
    public Task152(int capacity) {
        this.capacity = capacity;
        this.size = 0;
        this.table = new LinkedList[capacity];
        for (int i = 0; i < capacity; i++) {
            table[i] = new LinkedList<>();
        }
    }
    
    private int hash(String key) {
        return Math.abs(key.hashCode()) % capacity;
    }
    
    public void insert(String key, int value) {
        int index = hash(key);
        LinkedList<HashNode> bucket = table[index];
        
        for (HashNode node : bucket) {
            if (node.key.equals(key)) {
                node.value = value;
                return;
            }
        }
        
        bucket.add(new HashNode(key, value));
        size++;
    }
    
    public Integer search(String key) {
        int index = hash(key);
        LinkedList<HashNode> bucket = table[index];
        
        for (HashNode node : bucket) {
            if (node.key.equals(key)) {
                return node.value;
            }
        }
        
        return null;
    }
    
    public boolean delete(String key) {
        int index = hash(key);
        LinkedList<HashNode> bucket = table[index];
        
        for (HashNode node : bucket) {
            if (node.key.equals(key)) {
                bucket.remove(node);
                size--;
                return true;
            }
        }
        
        return false;
    }
    
    public int getSize() {
        return size;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Basic insert and search");
        Task152 ht1 = new Task152(10);
        ht1.insert("apple", 100);
        ht1.insert("banana", 200);
        ht1.insert("orange", 300);
        System.out.println("Search 'apple': " + ht1.search("apple"));
        System.out.println("Search 'banana': " + ht1.search("banana"));
        System.out.println("Search 'orange': " + ht1.search("orange"));
        
        System.out.println("\\nTest Case 2: Delete operation");
        Task152 ht2 = new Task152(10);
        ht2.insert("cat", 50);
        ht2.insert("dog", 60);
        System.out.println("Before delete - Search 'cat': " + ht2.search("cat"));
        ht2.delete("cat");
        System.out.println("After delete - Search 'cat': " + ht2.search("cat"));
        System.out.println("Search 'dog': " + ht2.search("dog"));
        
        System.out.println("\\nTest Case 3: Update existing key");
        Task152 ht3 = new Task152(10);
        ht3.insert("key1", 10);
        System.out.println("Initial value: " + ht3.search("key1"));
        ht3.insert("key1", 20);
        System.out.println("Updated value: " + ht3.search("key1"));
        
        System.out.println("\\nTest Case 4: Search non-existent key");
        Task152 ht4 = new Task152(10);
        ht4.insert("exist", 100);
        System.out.println("Search 'exist': " + ht4.search("exist"));
        System.out.println("Search 'notexist': " + ht4.search("notexist"));
        
        System.out.println("\\nTest Case 5: Multiple operations");
        Task152 ht5 = new Task152(10);
        ht5.insert("a", 1);
        ht5.insert("b", 2);
        ht5.insert("c", 3);
        System.out.println("Size: " + ht5.getSize());
        ht5.delete("b");
        System.out.println("After delete, Size: " + ht5.getSize());
        System.out.println("Search 'a': " + ht5.search("a"));
        System.out.println("Search 'b': " + ht5.search("b"));
        System.out.println("Search 'c': " + ht5.search("c"));
    }
}
