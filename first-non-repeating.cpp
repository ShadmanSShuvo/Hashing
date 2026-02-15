#include <bits/stdc++.h>
using namespace std;

const int MAX_CHAR = 26;  

char nonRep(const string& s) {
    vector<int> vis(MAX_CHAR, -1);
    for (int i = 0; i < s.length(); ++i) {
        int index = s[i] - 'a';
        if (vis[index] == -1) {
            
            // Store the index when character is first seen
            vis[index] = i;  
        } else {
            
            // Mark character as repeated
            vis[index] = -2; 
        }
    }

    int idx = -1;

    // Find the smallest index of the non-repeating characters
    for (int i = 0; i < MAX_CHAR; ++i) {
        if (vis[i] >= 0 && (idx == -1 || vis[i] < vis[idx])) {
            idx = i;
        }
    }
    return (idx == -1) ? '$' : s[vis[idx]];
}

int main() {
    string s = "aabbccc";
    cout << nonRep(s) << endl;
    return 0;
}