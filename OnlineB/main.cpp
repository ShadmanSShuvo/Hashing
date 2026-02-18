#include "HashTable.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

int main() {
    const int NUM_WORDS = 10000;
    const int WORD_LENGTH = 10;

    cout << "=== Hash Table Demonstration Setup ===" << endl;
    cout << "Generating " << NUM_WORDS << " unique " << WORD_LENGTH
         << "-letter words..." << endl;

    WordGenerator generator;
    vector<string> words;
    words.reserve(NUM_WORDS);
    for (int i = 0; i < NUM_WORDS; i++) {
        words.push_back(generator.generateWord(WORD_LENGTH));
    }
    cout << "Generated " << words.size() << " unique words." << endl << endl;

    // --- PART A: Probe Sequence Demo [cite: 6] ---
    cout << "=== Probe Sequence Demo (Double Hashing) ===" << endl;

    // Create a table specifically for this demo using Double Hashing (Method 1)
    // and Hash Function 1
    HashTable<int> demoTable(DOUBLE_HASHING, 1);

    // Insert the 10,000 words [cite: 6]
    cout << "Inserting words into demo table..." << endl;
    for (int i = 0; i < NUM_WORDS; i++) {
        demoTable.insert(words[i], i + 1);
    }
    cout << "Insertion complete." << endl;

    // Interactive Input for Probe Sequence [cite: 7, 8, 9]
    int n;
    cout << "\nEnter number of keys to probe (n): ";
    if (cin >> n) {
        cout << "Enter " << n << " keys (one per line):" << endl;
        for (int i = 0; i < n; i++) {
            string key;
            cin >> key;
            demoTable.printProbeSequence(key);
        }
    }

    return 0;
}