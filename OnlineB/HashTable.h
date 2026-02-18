#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>

using namespace std;

// Configuration parameters
const int INITIAL_TABLE_SIZE = 13;
const double LOAD_FACTOR_THRESHOLD = 0.5;
const double COMPACTION_THRESHOLD = 0.25;
// Constants for custom probing
const int C1 = 1;
const int C2 = 3;

// Enum for collision resolution methods
enum CollisionMethod { CHAINING, DOUBLE_HASHING, CUSTOM_PROBING };

// Node structure for chaining
template <typename V> struct ChainNode {
    string key;
    V value;
    ChainNode *next;
    ChainNode(const string &k, const V &v) : key(k), value(v), next(nullptr) {}
};

// Entry structure for open addressing
template <typename V> struct Entry {
    string key;
    V value;
    bool occupied;
    bool deleted;
    Entry() : occupied(false), deleted(false) {}
    Entry(const string &k, const V &v)
        : key(k), value(v), occupied(true), deleted(false) {}
};

// Hash Table Class
template <typename V> class HashTable {
  private:
    int tableSize;
    int numElements;
    CollisionMethod method;
    int hashFunctionType; // 1 or 2

    // For chaining
    vector<ChainNode<V> *> chainTable;

    // For open addressing
    vector<Entry<V>> openTable;

    // Statistics
    long long totalCollisions;
    long long totalProbes;
    long long searchOperations;

    // For dynamic resizing
    int insertionsSinceExpansion;
    int deletionsSinceCompaction;
    int elementsAtLastResize;

    // Prime number utilities
    bool isPrime(int n) {
        if (n <= 1)
            return false;
        if (n <= 3)
            return true;
        if (n % 2 == 0 || n % 3 == 0)
            return false;
        for (int i = 5; i * i <= n; i += 6) {
            if (n % i == 0 || n % (i + 2) == 0)
                return false;
        }
        return true;
    }

    int nextPrime(int n) {
        if (n <= 2)
            return 2;
        int prime = (n % 2 == 0) ? n + 1 : n + 2;
        while (!isPrime(prime))
            prime += 2;
        return prime;
    }

    int prevPrime(int n) {
        if (n <= INITIAL_TABLE_SIZE)
            return INITIAL_TABLE_SIZE;
        int prime = (n % 2 == 0) ? n - 1 : n - 2;
        while (prime > INITIAL_TABLE_SIZE && !isPrime(prime))
            prime -= 2;
        return (prime < INITIAL_TABLE_SIZE) ? INITIAL_TABLE_SIZE : prime;
    }

    // Hash functions
    int hash1(const string &key) {
        unsigned long long hashValue = 0;
        const int p = 31;
        const int m = tableSize;
        unsigned long long p_pow = 1;
        for (char c : key) {
            hashValue = (hashValue + (c - 'a' + 1) * p_pow) % m;
            p_pow = (p_pow * p) % m;
        }
        return (int)hashValue;
    }

    int hash2(const string &key) {
        unsigned long long hash = 5381;
        for (char c : key) {
            hash = ((hash << 5) + hash) + c;
        }
        return (int)(hash % tableSize);
    }

    int getHash(const string &key) {
        return (hashFunctionType == 1) ? hash1(key) : hash2(key);
    }

    int auxHash(const string &key) {
        // Aux hash must be independent of Primary hash
        int h_val = (hashFunctionType == 1) ? hash2(key) : hash1(key);
        return 1 + (h_val % (tableSize - 1));
    }

    // Probing Index Calculators
    int getDoubleHash(const string &key, int i) {
        // (Hash(k) + i * auxHash(k)) % N
        long long h = getHash(key);
        long long aux = auxHash(key);
        return (int)((h + (long long)i * aux) % tableSize);
    }

    int getCustomHash(const string &key, int i) {
        // (Hash(k) + C1*i*auxHash(k) + C2*i^2) % N
        long long h = getHash(key);
        long long aux = auxHash(key);
        long long result =
            (h + C1 * (long long)i * aux + C2 * (long long)i * i) % tableSize;
        return (int)((result >= 0) ? result : result + tableSize);
    }

    double getLoadFactor() { return (double)numElements / tableSize; }

    // Helper for internal insert (used by rehash)
    bool insertInternal(const string &key, const V &value, bool isRehashing) {
        if (method == CHAINING) {
            int index = getHash(key);

            if (chainTable[index] != nullptr) {
                totalCollisions++;
                ChainNode<V> *current = chainTable[index];
                while (current != nullptr) {
                    if (current->key == key)
                        return false;
                    current = current->next;
                }
            }

            ChainNode<V> *newNode = new ChainNode<V>(key, value);
            newNode->next = chainTable[index];
            chainTable[index] = newNode;
        } else {
            int i = 0;
            int index;
            while (i < tableSize) {
                if (method == DOUBLE_HASHING)
                    index = getDoubleHash(key, i);
                else
                    index = getCustomHash(key, i);

                if (!openTable[index].occupied || openTable[index].deleted) {
                    openTable[index] = Entry<V>(key, value);
                    break;
                }

                if (openTable[index].occupied && !openTable[index].deleted &&
                    openTable[index].key == key) {
                    return false;
                }

                totalCollisions++;
                i++;
            }
            if (i == tableSize)
                return false;
        }

        numElements++;
        if (!isRehashing) {
            insertionsSinceExpansion++;
            checkAndResize();
        }
        return true;
    }

    void checkAndResize() {
        double loadFactor = getLoadFactor();
        if (loadFactor > LOAD_FACTOR_THRESHOLD &&
            insertionsSinceExpansion >= elementsAtLastResize / 2) {
            rehash(nextPrime(2 * tableSize));
        } else if (loadFactor < COMPACTION_THRESHOLD &&
                   tableSize > INITIAL_TABLE_SIZE &&
                   deletionsSinceCompaction >= elementsAtLastResize / 2) {
            int newSize = prevPrime(tableSize / 2);
            if (newSize < INITIAL_TABLE_SIZE)
                newSize = INITIAL_TABLE_SIZE;
            rehash(newSize);
        }
    }

    void rehash(int newSize) {
        int oldSize = tableSize;
        vector<ChainNode<V> *> oldChainTable = chainTable;
        vector<Entry<V>> oldOpenTable = openTable;

        tableSize = newSize;
        numElements = 0;
        insertionsSinceExpansion = 0;
        deletionsSinceCompaction = 0;

        if (method == CHAINING) {
            chainTable.clear();
            chainTable.resize(tableSize, nullptr);
            for (int i = 0; i < oldSize; i++) {
                ChainNode<V> *current = oldChainTable[i];
                while (current != nullptr) {
                    insertInternal(current->key, current->value, true);
                    ChainNode<V> *temp = current;
                    current = current->next;
                    delete temp;
                }
            }
        } else {
            openTable.clear();
            openTable.resize(tableSize);
            for (int i = 0; i < oldSize; i++) {
                if (oldOpenTable[i].occupied && !oldOpenTable[i].deleted) {
                    insertInternal(oldOpenTable[i].key, oldOpenTable[i].value,
                                   true);
                }
            }
        }
        elementsAtLastResize = numElements;
    }

  public:
    HashTable(CollisionMethod m, int hashType)
        : tableSize(INITIAL_TABLE_SIZE), numElements(0), method(m),
          hashFunctionType(hashType), totalCollisions(0), totalProbes(0),
          searchOperations(0), insertionsSinceExpansion(0),
          deletionsSinceCompaction(0), elementsAtLastResize(0) {

        if (method == CHAINING) {
            chainTable.resize(tableSize, nullptr);
        } else {
            openTable.resize(tableSize);
        }
    }

    ~HashTable() {
        if (method == CHAINING) {
            for (auto node : chainTable) {
                while (node) {
                    ChainNode<V> *temp = node;
                    node = node->next;
                    delete temp;
                }
            }
        }
    }

    bool insert(const string &key, const V &value) {
        return insertInternal(key, value, false);
    }

    bool search(const string &key, V &value) {
        searchOperations++;
        int probes = 0;

        if (method == CHAINING) {
            int index = getHash(key);
            probes++;
            ChainNode<V> *current = chainTable[index];
            while (current != nullptr) {
                if (current->key == key) {
                    value = current->value;
                    totalProbes += probes;
                    return true;
                }
                current = current->next;
                probes++;
            }
        } else {
            int i = 0;
            int index;
            while (i < tableSize) {
                if (method == DOUBLE_HASHING)
                    index = getDoubleHash(key, i);
                else
                    index = getCustomHash(key, i);

                probes++;

                if (!openTable[index].occupied)
                    break;

                if (openTable[index].occupied && !openTable[index].deleted &&
                    openTable[index].key == key) {
                    value = openTable[index].value;
                    totalProbes += probes;
                    return true;
                }
                i++;
            }
        }
        totalProbes += probes;
        return false;
    }

     // --- NEW: Print Probe Sequence Method [cite: 3, 4] ---
        void
        printProbeSequence(const string &key) {
            if (method == CHAINING) {
                cout << "Probe sequence not supported for Chaining." << endl;
                return;
            }

            int i = 0;
            int index;
            bool first = true;

            // Traverse using the same probing logic as search/insert
            while (i < tableSize) {
                if (method == DOUBLE_HASHING)
                    index = getDoubleHash(key, i);
                else
                    index = getCustomHash(key, i);

                if (!first)
                    cout << " -> ";
                cout << index;
                first = false;

                // Stop Condition 1: Key found
                if (openTable[index].occupied && !openTable[index].deleted &&
                    openTable[index].key == key) {
                    break;
                }

                // Stop Condition 2: Empty slot found (Key not in table)
                if (!openTable[index].occupied) {
                    break;
                }

                // Otherwise, collision occurred (or DELETED slot), continue to
                // next probe
                i++;
            }
            cout << endl;
        }

    bool remove(const string &key) {
        // Implementation omitted for brevity
        return false;
    }

    long long getCollisions() const { return totalCollisions; }

    double getAverageProbes() const {
        return searchOperations > 0 ? (double)totalProbes / searchOperations
                                    : 0.0;
    }

    void resetStatistics() {
        totalCollisions = 0;
        totalProbes = 0;
        searchOperations = 0;
    }
};

// Random Word Generator Class
class WordGenerator {
  private:
    mt19937 gen;
    uniform_int_distribution<> dis;
    set<string> generatedWords;

  public:
    WordGenerator() : gen(42), dis(0, 25) {}

    string generateWord(int length) {
        string word;
        while (true) {
            word = "";
            for (int i = 0; i < length; i++) {
                word += char('a' + dis(gen));
            }
            if (generatedWords.find(word) == generatedWords.end()) {
                generatedWords.insert(word);
                return word;
            }
        }
    }

    void reset() { generatedWords.clear(); }
};

#endif // HASHTABLE_H