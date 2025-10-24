
import java.util.LinkedList;

public class Task152 {
    private static class HashNode<K, V> {
        K key;
        V value;
        
        public HashNode(K key, V value) {
            this.key = key;
            this.value = value;
        }
    }
    
    private static class HashTable<K, V> {
        private LinkedList<HashNode<K, V>>[] buckets;
        private int capacity;
        private int size;
        
        @SuppressWarnings("unchecked")
        public HashTable(int capacity) {
            this.capacity = capacity;
            this.size = 0;
            this.buckets = new LinkedList[capacity];
            for (int i = 0; i < capacity; i++) {
                buckets[i] = new LinkedList<>();
            }
        }
        
        private int getBucketIndex(K key) {
            int hashCode = key.hashCode();
            return Math.abs(hashCode) % capacity;
        }
        
        public void insert(K key, V value) {
            int bucketIndex = getBucketIndex(key);
            LinkedList<HashNode<K, V>> bucket = buckets[bucketIndex];
            
            for (HashNode<K, V> node : bucket) {
                if (node.key.equals(key)) {
                    node.value = value;
                    return;
                }
            }
            
            bucket.add(new HashNode<>(key, value));
            size++;
        }
        
        public V search(K key) {
            int bucketIndex = getBucketIndex(key);
            LinkedList<HashNode<K, V>> bucket = buckets[bucketIndex];
            
            for (HashNode<K, V> node : bucket) {
                if (node.key.equals(key)) {
                    return node.value;
                }
            }
            
            return null;
        }
        
        public boolean delete(K key) {
            int bucketIndex = getBucketIndex(key);
            LinkedList<HashNode<K, V>> bucket = buckets[bucketIndex];
            
            for (HashNode<K, V> node : bucket) {
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
        
        public boolean isEmpty() {
            return size == 0;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Basic Insert and Search");
        HashTable<String, Integer> ht1 = new HashTable<>(10);
        ht1.insert("apple", 5);
        ht1.insert("banana", 10);
        ht1.insert("orange", 15);
        System.out.println("Search 'apple': " + ht1.search("apple"));
        System.out.println("Search 'banana': " + ht1.search("banana"));
        System.out.println("Size: " + ht1.getSize());
        
        System.out.println("\\nTest Case 2: Delete Operation");
        HashTable<String, Integer> ht2 = new HashTable<>(10);
        ht2.insert("key1", 100);
        ht2.insert("key2", 200);
        System.out.println("Before delete - Size: " + ht2.getSize());
        System.out.println("Delete 'key1': " + ht2.delete("key1"));
        System.out.println("After delete - Size: " + ht2.getSize());
        System.out.println("Search 'key1': " + ht2.search("key1"));
        
        System.out.println("\\nTest Case 3: Update Existing Key");
        HashTable<String, Integer> ht3 = new HashTable<>(10);
        ht3.insert("counter", 1);
        System.out.println("Initial value: " + ht3.search("counter"));
        ht3.insert("counter", 2);
        System.out.println("Updated value: " + ht3.search("counter"));
        System.out.println("Size: " + ht3.getSize());
        
        System.out.println("\\nTest Case 4: Search Non-existent Key");
        HashTable<String, Integer> ht4 = new HashTable<>(10);
        ht4.insert("exists", 99);
        System.out.println("Search 'exists': " + ht4.search("exists"));
        System.out.println("Search 'notexists': " + ht4.search("notexists"));
        
        System.out.println("\\nTest Case 5: Multiple Operations");
        HashTable<Integer, String> ht5 = new HashTable<>(10);
        ht5.insert(1, "one");
        ht5.insert(2, "two");
        ht5.insert(3, "three");
        System.out.println("Search 2: " + ht5.search(2));
        ht5.delete(2);
        System.out.println("After delete 2: " + ht5.search(2));
        ht5.insert(4, "four");
        System.out.println("Search 4: " + ht5.search(4));
        System.out.println("Final size: " + ht5.getSize());
    }
}
