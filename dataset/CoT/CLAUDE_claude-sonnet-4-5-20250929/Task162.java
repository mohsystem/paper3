
import java.util.*;

public class Task162 {
    static class Edge {
        int destination;
        int weight;
        
        public Edge(int destination, int weight) {
            this.destination = destination;
            this.weight = weight;
        }
    }
    
    static class Node implements Comparable<Node> {
        int vertex;
        int distance;
        
        public Node(int vertex, int distance) {
            this.vertex = vertex;
            this.distance = distance;
        }
        
        @Override
        public int compareTo(Node other) {
            return Integer.compare(this.distance, other.distance);
        }
    }
    
    public static Map<Integer, Integer> dijkstra(Map<Integer, List<Edge>> graph, int start) {
        if (graph == null || !graph.containsKey(start)) {
            return new HashMap<>();
        }
        
        Map<Integer, Integer> distances = new HashMap<>();
        Set<Integer> visited = new HashSet<>();
        PriorityQueue<Node> pq = new PriorityQueue<>();
        
        // Initialize all distances to infinity
        for (Integer vertex : graph.keySet()) {
            distances.put(vertex, Integer.MAX_VALUE);
        }
        distances.put(start, 0);
        pq.offer(new Node(start, 0));
        
        while (!pq.isEmpty()) {
            Node current = pq.poll();
            int currentVertex = current.vertex;
            
            if (visited.contains(currentVertex)) {
                continue;
            }
            visited.add(currentVertex);
            
            if (!graph.containsKey(currentVertex)) {
                continue;
            }
            
            for (Edge edge : graph.get(currentVertex)) {
                if (edge.weight < 0) {
                    continue; // Skip negative weights
                }
                
                int neighbor = edge.destination;
                int newDistance = distances.get(currentVertex);
                
                // Check for integer overflow
                if (newDistance != Integer.MAX_VALUE && 
                    newDistance + edge.weight < Integer.MAX_VALUE) {
                    newDistance += edge.weight;
                    
                    if (newDistance < distances.get(neighbor)) {
                        distances.put(neighbor, newDistance);
                        pq.offer(new Node(neighbor, newDistance));
                    }
                }
            }
        }
        
        return distances;
    }
    
    public static void main(String[] args) {
        // Test Case 1: Simple graph
        Map<Integer, List<Edge>> graph1 = new HashMap<>();
        graph1.put(0, Arrays.asList(new Edge(1, 4), new Edge(2, 1)));
        graph1.put(1, Arrays.asList(new Edge(3, 1)));
        graph1.put(2, Arrays.asList(new Edge(1, 2), new Edge(3, 5)));
        graph1.put(3, new ArrayList<>());
        
        System.out.println("Test Case 1:");
        System.out.println(dijkstra(graph1, 0));
        
        // Test Case 2: Disconnected graph
        Map<Integer, List<Edge>> graph2 = new HashMap<>();
        graph2.put(0, Arrays.asList(new Edge(1, 2)));
        graph2.put(1, new ArrayList<>());
        graph2.put(2, Arrays.asList(new Edge(3, 1)));
        graph2.put(3, new ArrayList<>());
        
        System.out.println("\\nTest Case 2:");
        System.out.println(dijkstra(graph2, 0));
        
        // Test Case 3: Single node
        Map<Integer, List<Edge>> graph3 = new HashMap<>();
        graph3.put(0, new ArrayList<>());
        
        System.out.println("\\nTest Case 3:");
        System.out.println(dijkstra(graph3, 0));
        
        // Test Case 4: Multiple paths
        Map<Integer, List<Edge>> graph4 = new HashMap<>();
        graph4.put(0, Arrays.asList(new Edge(1, 10), new Edge(2, 5)));
        graph4.put(1, Arrays.asList(new Edge(3, 1)));
        graph4.put(2, Arrays.asList(new Edge(1, 3), new Edge(3, 9)));
        graph4.put(3, new ArrayList<>());
        
        System.out.println("\\nTest Case 4:");
        System.out.println(dijkstra(graph4, 0));
        
        // Test Case 5: Larger graph
        Map<Integer, List<Edge>> graph5 = new HashMap<>();
        graph5.put(0, Arrays.asList(new Edge(1, 7), new Edge(2, 9), new Edge(5, 14)));
        graph5.put(1, Arrays.asList(new Edge(0, 7), new Edge(2, 10), new Edge(3, 15)));
        graph5.put(2, Arrays.asList(new Edge(0, 9), new Edge(1, 10), new Edge(3, 11), new Edge(5, 2)));
        graph5.put(3, Arrays.asList(new Edge(1, 15), new Edge(2, 11), new Edge(4, 6)));
        graph5.put(4, Arrays.asList(new Edge(3, 6), new Edge(5, 9)));
        graph5.put(5, Arrays.asList(new Edge(0, 14), new Edge(2, 2), new Edge(4, 9)));
        
        System.out.println("\\nTest Case 5:");
        System.out.println(dijkstra(graph5, 0));
    }
}
