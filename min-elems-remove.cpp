// CPP program to find minimum elements to remove 
// so no common element exists in both arrays using a single map

#include <bits/stdc++.h>
using namespace std;

int minRemove(vector<int>& arr1, vector<int>& arr2) {
    unordered_map<int, int> count;

    // Count elements of arr1
    for (int x : arr1) {
        count[x] += 1;
    }

    int res = 0;
    
    // Count elements of arr2 and check common elements
    for (int x : arr2) {
        if (count.find(x) != count.end() && count[x] > 0) {
            count[x]--;
            res++;
        }
    }

    return res;
}

int main() {
    vector<int> arr1 = {1, 2, 3, 4};
    vector<int> arr2 = {2, 3, 4, 5, 8};
    
    cout << minRemove(arr1, arr2);
    
    return 0;
}