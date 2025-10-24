
import random
import math

class Solution:
    def __init__(self, radius: float, x_center: float, y_center: float):
        """\n        Initialize the Solution with circle parameters.\n        Security: Validate input parameters to prevent invalid state.\n        """
        # Input validation: ensure parameters meet constraints
        if radius <= 0.0 or radius > 1e8:
            raise ValueError("Radius must be positive and <= 10^8")
        
        if x_center < -1e7 or x_center > 1e7 or y_center < -1e7 or y_center > 1e7:
            raise ValueError("Center coordinates must be in [-10^7, 10^7]")
        
        self.radius = radius
        self.x_center = x_center
        self.y_center = y_center
    
    def randPoint(self):
        """\n        Generate a uniform random point inside the circle.\n        Security: Use secrets module for better randomness if needed for security-critical apps.\n        Returns: List[float] containing [x, y] coordinates.\n        """
        # Generate random angle in [0, 2*PI)
        angle = random.uniform(0, 2 * math.pi)
        
        # Generate random radius with uniform distribution
        # Use sqrt to ensure uniform area distribution
        r = math.sqrt(random.uniform(0, 1)) * self.radius
        
        # Convert polar to Cartesian coordinates
        # Bounds are safe as r <= radius and angle is normalized
        x = self.x_center + r * math.cos(angle)
        y = self.y_center + r * math.sin(angle)
        
        return [x, y]


# Test driver with 5 test cases
if __name__ == "__main__":
    print("Test Case 1: Circle with radius 1.0, center (0.0, 0.0)")
    try:
        sol1 = Solution(1.0, 0.0, 0.0)
        for i in range(3):
            point = sol1.randPoint()
            print(f"  Point {i + 1}: [{point[0]:.5f}, {point[1]:.5f}]")
    except ValueError as e:
        print(f"  Error: {e}")
    
    print("\\nTest Case 2: Circle with radius 10.0, center (5.0, -7.5)")
    try:
        sol2 = Solution(10.0, 5.0, -7.5)
        for i in range(3):
            point = sol2.randPoint()
            print(f"  Point {i + 1}: [{point[0]:.5f}, {point[1]:.5f}]")
    except ValueError as e:
        print(f"  Error: {e}")
    
    print("\\nTest Case 3: Circle with radius 0.01, center (0.0, 0.0)")
    try:
        sol3 = Solution(0.01, 0.0, 0.0)
        for i in range(3):
            point = sol3.randPoint()
            print(f"  Point {i + 1}: [{point[0]:.7f}, {point[1]:.7f}]")
    except ValueError as e:
        print(f"  Error: {e}")
    
    print("\\nTest Case 4: Circle with radius 100.0, center (-50.0, 50.0)")
    try:
        sol4 = Solution(100.0, -50.0, 50.0)
        for i in range(3):
            point = sol4.randPoint()
            print(f"  Point {i + 1}: [{point[0]:.5f}, {point[1]:.5f}]")
    except ValueError as e:
        print(f"  Error: {e}")
    
    print("\\nTest Case 5: Invalid input (negative radius)")
    try:
        sol5 = Solution(-1.0, 0.0, 0.0)
        print("  Should have thrown exception")
    except ValueError as e:
        print(f"  Correctly rejected: {e}")
