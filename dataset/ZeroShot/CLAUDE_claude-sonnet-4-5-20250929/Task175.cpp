
#include <iostream>
#include <vector>
using namespace std;

class MountainArray {
private:
    vector<int> arr;
public:
    MountainArray(vector<int> a) : arr(a) {}
    int get(int index) { return arr[index]; }
    int length() { return arr.size(); }
};

class Task175 {
public:
    static int findPeak(MountainArray& mountainArr, int length) {
        int left = 0, right = length - 1;
        while (left < right) {
            int mid = left + (right - left) / 2;
            if (mountainArr.get(mid) < mountainArr.get(mid + 1)) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }
        return left;
    }
    
    static int binarySearchAscending(MountainArray& mountainArr, int target, int left, int right) {
        while (left <= right) {
            int mid = left + (right - left) / 2;
            int midVal = mountainArr.get(mid);
            if (midVal == target) {
                return mid;
            } else if (midVal < target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return -1;
    }
    
    static int binarySearchDescending(MountainArray& mountainArr, int target, int left, int right) {
        while (left <= right) {
            int mid = left + (right - left) / 2;
            int midVal = mountainArr.get(mid);
            if (midVal == target) {
                return mid;
            } else if (midVal > target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return -1;
    }
    
    static int findInMountainArray(int target, MountainArray& mountainArr) {
        int length = mountainArr.length();
        int peakIndex = findPeak(mountainArr, length);
        int result = binarySearchAscending(mountainArr, target, 0, peakIndex);
        if (result != -1) {
            return result;
        }
        return binarySearchDescending(mountainArr, target, peakIndex + 1, length - 1);
    }
};

int main() {
    // Test case 1
    vector<int> arr1 = {1, 2, 3, 4, 5, 3, 1};
    MountainArray ma1(arr1);
    cout << "Test 1: " << Task175::findInMountainArray(3, ma1) << endl;
    
    // Test case 2
    vector<int> arr2 = {0, 1, 2, 4, 2, 1};
    MountainArray ma2(arr2);
    cout << "Test 2: " << Task175::findInMountainArray(3, ma2) << endl;
    
    // Test case 3
    vector<int> arr3 = {1, 2, 3, 4, 5, 3, 1};
    MountainArray ma3(arr3);
    cout << "Test 3: " << Task175::findInMountainArray(5, ma3) << endl;
    
    // Test case 4
    vector<int> arr4 = {1, 5, 2};
    MountainArray ma4(arr4);
    cout << "Test 4: " << Task175::findInMountainArray(2, ma4) << endl;
    
    // Test case 5
    vector<int> arr5 = {1, 2, 3, 4, 5, 3, 1};
    MountainArray ma5(arr5);
    cout << "Test 5: " << Task175::findInMountainArray(1, ma5) << endl;
    
    return 0;
}
