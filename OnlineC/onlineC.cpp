#include <iostream>
#include <vector>
#include <list>
#include <functional>
#include <string>
#include <random>
#include <algorithm>
#include <set>
#include <cmath>
#include <ctime>
using namespace std;

// ---------------- CONFIG ----------------
int INITIAL_SIZE = 13;
double LOAD_FACTOR_UPPER = 0.5;
double LOAD_FACTOR_LOWER = 0.25;

// ---------------- PRIME UTILS ----------------
bool isPrime(int n){
    if(n<=1) return false;
    if(n<=3) return true;
    if(n%2==0||n%3==0) return false;
    for(int i=5;i*i<=n;i+=6)
        if(n%i==0||n%(i+2)==0) return false;
    return true;
}
int nextPrime(int n){ while(!isPrime(n)) n++; return n; }
int prevPrime(int n){ n=max(2,n); while(!isPrime(n)) n--; return n; }

// ---------------- HASH FUNCTIONS ----------------
size_t polyHash(const string &s, size_t p=31, size_t mod=1e9+9){
    size_t hash=0,power=1;
    for(char c:s){
        hash=(hash+(c-'a'+1)*power)%mod;
        power=(power*p)%mod;
    }
    return hash;
}

// size_t djb2Hash(const string &s){
//     size_t hash=5381;
//     for(char c:s) hash=((hash<<5)+hash)+c;
//     return hash;
// }

// ---------------- AUX HASH ----------------
// Template specialization for string
// template<typename K>
// size_t auxHashImpl(const K &key, size_t tableSize){
//     size_t h=hash<K>{}(key);
//     return 1 + (h % (tableSize-1));
// }

// Overload for string
// size_t auxHash(const string &key, size_t tableSize){
//     size_t h=hash<string>{}(key);
//     return 1 + (h % (tableSize-1));
// }

// Template version for other types
template<typename K>
size_t auxHash(const K &key, size_t tableSize){
    return auxHashImpl(key, tableSize);
}

// ---------------- KEY TO STRING ----------------
inline string keyToString(const string &s){ return s; }
template<typename K>
inline string keyToString(const K &key){ return to_string(key); }

// ---------------- ENTRY ----------------
template<typename K,typename V>
struct Entry{
    K key;
    V value;
    Entry(K k,V v):key(k),value(v){}
};

// ---------------- RANDOM WORD GENERATOR ----------------
string generateWord(int len, mt19937 &rng){
    uniform_int_distribution<int> dist('a','z');
    string w;
    for(int i=0;i<len;i++) w+=(char)dist(rng);
    return w;
}

// ---------------- CHAINING ----------------
template<typename K,typename V>
class HashTableChaining{
public:
    int size,nElements,lastExpansion,lastCompaction;
    vector<list<Entry<K,V>>> table;
    int collisionCount;

    HashTableChaining(int s=INITIAL_SIZE){
        size=s; nElements=0;
        collisionCount=0;
        table.resize(size);
        lastExpansion=lastCompaction=0;
    }

    void adjustSize(){
        double lf=(double)nElements/size;
        if(lf>LOAD_FACTOR_UPPER && nElements-lastExpansion>=size/2) expand();
        else if(lf<LOAD_FACTOR_LOWER && nElements-lastCompaction>=size/2) compact();
    }
    void expand(){ rehash(nextPrime(size*2+1)); lastExpansion=nElements; }
    void compact(){ if(size!=INITIAL_SIZE){ rehash(prevPrime(size/2)); lastCompaction=nElements; } }

    void rehash(int newSize){
        auto old=table;
        table.clear(); table.resize(newSize);
        size=newSize; nElements=0;
        for(auto &bucket:old)
            for(auto &e:bucket)
                insert(e.key,e.value);
    }

    bool insert(const K &key,const V &value){
        size_t idx=polyHash(keyToString(key))%size;
        for(auto &e:table[idx])
            if(e.key==key) return false;
        collisionCount+=table[idx].size();
        table[idx].emplace_back(key,value);
        nElements++;
        adjustSize();
        return true;
    }
    
    bool search(const K &key,int &hits){
        size_t idx=polyHash(keyToString(key))%size;
        hits=0;
        for(auto &e:table[idx]){
            hits++;
            if(e.key==key) return true;
        }
        return false;
    }

    bool remove(const K &key){
        size_t idx=polyHash(keyToString(key))%size;
        for(auto it=table[idx].begin();it!=table[idx].end();++it){
            if(it->key==key){
                table[idx].erase(it);
                nElements--;
                adjustSize();
                return true;
            }
        }
        return false;
    }
};

// ---------------- MAIN ----------------
int main(){
    int n;
    HashTableChaining<int, int> ht1, ht2;
    vector<int>Union, intersection , difference;
    cin>>n;
        int num;
        //////////////auto polyWrapper = [](const int &s) -> size_t { return polyHash(s); };
    for(int i=0; i<n; i++){
        cin>>num;
        ht1.insert(num, 1);
        Union.push_back(num);
    }
    cin>>n;
    int hits = 0;
    for(int i=0; i<n; i++){
        cin>>num;
        if(!ht1.insert(num, 1)) intersection.push_back(num);
        else {
            Union.push_back(num);
        }
        ht2.insert(num, 1);
    }
    for(int i=0; i<Union.size(); i++){
        if(!ht2.search(Union[i], hits)) difference.push_back(Union[i]); 
    }
    sort(Union.begin(), Union.end());
    sort(intersection.begin(), intersection.end());
    sort(difference.begin(), difference.end());
    cout<<"Intersection:";
    for(auto it: intersection)cout<<it<<" ";
    cout<<endl<<"Union:";
    for(auto it: Union)cout<<it<<" ";
    cout<<endl<<"Difference(A-B):";
    for(auto it: difference)cout<<it<<" ";
    cout<<endl;
    return 0;
}
