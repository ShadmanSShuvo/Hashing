// C++ Program to find the 
// duplicate element
#include <iostream>
#include <vector>
#include <unordered_set>
using namespace std;

int findDuplicate(vector<int>& arr) {

    // Create an unordered_set
    unordered_set<int> s;
    for (int x : arr) {

        // If the element is already in the set
        if (s.find(x) != s.end())
            return x;
        s.insert(x);
    }
    return -1;
}

int main() {
    vector<int> arr = {1, 3, 2, 3, 4};
    cout << findDuplicate(arr);
    return 0;
}