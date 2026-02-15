#include <bits/stdc++.h>
using namespace std;

bool isSubset( vector<int>& a,  vector<int>& b) {

    // Create a hash set and insert all elements of a
    multiset<int> hashSet(a.begin(), a.end());
    
    // Check each element of b in the hash set
    for (int num : b) {
        if (hashSet.find(num) == hashSet.end()) {
            return false;
        }
        hashSet.erase(hashSet.find(num));
    }
    
    // If all elements of b are found in the hash set
    return true;
}

int main() {
    vector<int> a = {11, 1, 13, 21, 3, 7};
    vector<int> b = {11, 3, 7, 1};
    
    if (isSubset(a, b)) {
        cout << "true" << endl;
    } else {
        cout << "false" << endl;
    }
    
    return 0;
}