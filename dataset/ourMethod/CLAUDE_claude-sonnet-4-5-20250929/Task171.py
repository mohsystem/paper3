
from typing import List

class Solution:
    def reversePairs(self, nums: List[int]) -> int:
        """\n        Count reverse pairs using merge sort\n        Security: Input validation, bounds checking, no overflow issues in Python\n        """
        # Input validation
        if not nums or len(nums) == 0:
            return 0
        
        if len(nums) > 50000:
            return 0
        
        # Create temporary array for merging
        temp = [0] * len(nums)
        
        return self._merge_sort_and_count(nums, temp, 0, len(nums) - 1)
    
    def _merge_sort_and_count(self, nums: List[int], temp: List[int], left: int, right: int) -> int:
        """\n        Recursive merge sort that counts reverse pairs\n        Security: Validates bounds at each step\n        """
        # Boundary check
        if left >= right:
            return 0
        
        # Calculate mid point (no overflow in Python)
        mid = left + (right - left) // 2
        
        count = 0
        count += self._merge_sort_and_count(nums, temp, left, mid)
        count += self._merge_sort_and_count(nums, temp, mid + 1, right)
        count += self._merge_and_count(nums, temp, left, mid, right)
        
        return count
    
    def _merge_and_count(self, nums: List[int], temp: List[int], left: int, mid: int, right: int) -> int:
        """\n        Merge helper that counts cross-boundary reverse pairs\n        Security: All array accesses are bounds-checked by Python\n        """
        # Input validation
        if left < 0 or mid < left or right < mid or right >= len(nums):
            return 0
        
        count = 0
        
        # Count reverse pairs where i is in left half and j is in right half
        # Python handles large integers natively, no overflow
        j = mid + 1
        for i in range(left, mid + 1):
            while j <= right and nums[i] > 2 * nums[j]:
                j += 1
            count += (j - (mid + 1))
        
        # Merge the two sorted halves
        i = left
        j = mid + 1
        k = left
        
        while i <= mid and j <= right:
            if nums[i] <= nums[j]:
                temp[k] = nums[i]
                i += 1
            else:
                temp[k] = nums[j]
                j += 1
            k += 1
        
        # Copy remaining elements
        while i <= mid:
            temp[k] = nums[i]
            i += 1
            k += 1
        
        while j <= right:
            temp[k] = nums[j]
            j += 1
            k += 1
        
        # Copy back to original array
        for idx in range(left, right + 1):
            nums[idx] = temp[idx]
        
        return count


# Test driver with 5 test cases
def main():
    solution = Solution()
    
    # Test case 1: Example 1 from problem
    test1 = [1, 3, 2, 3, 1]
    print(f"Test 1: {test1} -> {solution.reversePairs(test1)}")
    
    # Test case 2: Example 2 from problem
    test2 = [2, 4, 3, 5, 1]
    print(f"Test 2: {test2} -> {solution.reversePairs(test2)}")
    
    # Test case 3: Single element
    test3 = [1]
    print(f"Test 3: {test3} -> {solution.reversePairs(test3)}")
    
    # Test case 4: Already sorted
    test4 = [1, 2, 3, 4, 5]
    print(f"Test 4: {test4} -> {solution.reversePairs(test4)}")
    
    # Test case 5: With negative numbers and edge values
    test5 = [2147483647, -2147483648, 0, 1]
    print(f"Test 5: {test5} -> {solution.reversePairs(test5)}")


if __name__ == "__main__":
    main()
