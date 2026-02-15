// C++ program for intersection of two arrays with
// distinct elements using hash set

#include <iostream>
#include <vector>
#include <unordered_set>
using namespace std;

vector<int> intersect(vector<int>& a, vector<int>& b) {
  
    // Put all elements of a[] in hash set
    unordered_set<int> st(a.begin(), a.end());  
    vector<int> res;                            
    for (int i = 0; i < b.size(); i++) {
      
        // If the element is in st
        // then add it to result array
        if (st.find(b[i]) != st.end()) {
            res.push_back(b[i]); 
        }
    }

    return res;
}

int main() {
    vector<int> a = {5, 6, 2, 1, 4}; 
    vector<int> b = {7, 9, 4, 2};

    vector<int> res = intersect(a, b);
    for (int i = 0; i < res.size(); i++) 
        cout << res[i] << " ";

    return 0;
}