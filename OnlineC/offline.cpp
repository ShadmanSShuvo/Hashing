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

size_t djb2Hash(const string &s){
    size_t hash=5381;
    for(char c:s) hash=((hash<<5)+hash)+c;
    return hash;
}

// ---------------- AUX HASH ----------------
// Template specialization for string
template<typename K>
size_t auxHashImpl(const K &key, size_t tableSize){
    size_t h=hash<K>{}(key);
    return 1 + (h % (tableSize-1));
}

// Overload for string
size_t auxHash(const string &key, size_t tableSize){
    size_t h=hash<string>{}(key);
    return 1 + (h % (tableSize-1));
}

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
        

        auto polyWrapper = [](const string &s) -> size_t { return polyHash(s); };
        
        for(auto &bucket:old)
            for(auto &e:bucket)
                insert(e.key,e.value,polyWrapper);
    }

    bool insert(const K &key,const V &value,function<size_t(const string&)> hashFunc){
        size_t idx=hashFunc(keyToString(key))%size;
        for(auto &e:table[idx])
            if(e.key==key) return false;
        collisionCount+=table[idx].size();
        table[idx].emplace_back(key,value);
        nElements++;
        adjustSize();
        return true;
    }

    V search(const K &key,int &hits,function<size_t(const string&)> hashFunc){
        size_t idx=hashFunc(keyToString(key))%size;
        hits=0;
        for(auto &e:table[idx]){
            hits++;
            if(e.key==key) return e.value;
        }
        return V();
    }

    bool remove(const K &key,function<size_t(const string&)> hashFunc){
        size_t idx=hashFunc(keyToString(key))%size;
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

// ---------------- DOUBLE HASHING ----------------
template<typename K,typename V>
class HashTableDouble{
public:
    int size,nElements,lastExpansion,lastCompaction,collisionCount;
    vector<Entry<K,V>*> table;
    vector<bool> deleted;

    HashTableDouble(int s=INITIAL_SIZE){
        size=s;nElements=0;
        collisionCount=0;
        table.resize(size,nullptr);
        deleted.resize(size,false);
        lastExpansion=lastCompaction=0;
    }

    void adjustSize(){
        double lf=(double)nElements/size;
        //cout<<lf<<endl;
        if(lf>LOAD_FACTOR_UPPER && nElements-lastExpansion>=size/2) expand();
        else if(lf<LOAD_FACTOR_LOWER && nElements-lastCompaction>=size/2) compact();
    }

    void expand(){ rehash(nextPrime(size*2+1)); lastExpansion=nElements; }
    void compact(){ if(size!=INITIAL_SIZE){ rehash(prevPrime(size/2)); lastCompaction=nElements; } }

    void rehash(int newSize){
        auto old=table; auto oldDel=deleted;
        table.clear(); deleted.clear();
        table.resize(newSize,nullptr); deleted.resize(newSize,false);
        size=newSize; nElements=0;
        //cout<<"Current size:"<<size<<endl;

        auto polyWrapper = [](const string &s) -> size_t { return polyHash(s); };
        
        for(int i=0;i<(int)old.size();i++)
            if(old[i] && !oldDel[i]) insert(old[i]->key,old[i]->value,polyWrapper);
    }

    bool insert(const K &key,const V &value,function<size_t(const string&)> hashFunc){
        size_t h1=hashFunc(keyToString(key))%size;
        size_t h2=auxHash(key,size);
        int firstdel = -1;
        for(int i=0;i<size;i++){
            size_t idx=(h1+i*h2)%size;
            if(deleted[idx]){
                if(table[idx] && firstdel == -1) { 
                    firstdel = idx;
                }
            }
            if(!table[idx] && !deleted[idx]){
                if(firstdel != -1) idx = firstdel;
                table[idx]=new Entry<K,V>(key,value);
                deleted[idx]=false;
                nElements++;
                adjustSize();
                return true;
            } 
            else if(table[idx] && table[idx]->key==key) return false;
            collisionCount++;
        }
        return false;
    }

    V search(const K &key,int &hits,function<size_t(const string&)> hashFunc){
        size_t h1=hashFunc(keyToString(key))%size;
        size_t h2=auxHash(key,size);
        hits=0;
        for(int i=0;i<size;i++){
            size_t idx=(h1+i*h2)%size;
            hits++;
            if(!table[idx] && !deleted[idx]) return V();
            if(table[idx] && !deleted[idx] && table[idx]->key==key) return table[idx]->value;
        }
        return V();
    }

    bool remove(const K &key,function<size_t(const string&)> hashFunc){
        size_t h1=hashFunc(keyToString(key))%size;
        size_t h2=auxHash(key,size);
        for(int i=0;i<size;i++){
            size_t idx=(h1+i*h2)%size;
            if(!table[idx] && !deleted[idx]) return false;
            if(table[idx] && !deleted[idx] && table[idx]->key==key){
                deleted[idx]=true;
                nElements--;
                adjustSize();
                return true;
            }
        }
        return false;
    }
    void printProbeSequence(const K &key, int &hits,function<size_t(const string&)> hashFunc){
        size_t h1=hashFunc(keyToString(key))%size;
        size_t h2=auxHash(key,size);
        hits=0;
        vector<long long> probes;
        for(int i=0;i<size;i++){
            size_t idx=(h1+i*h2)%size;
            hits++;
            probes.push_back(idx);
            if(!table[idx] && !deleted[idx]) break;
            if(table[idx] && !deleted[idx] && table[idx]->key==key) break;
        }
        if(probes.size()==0) return;
        for(long long i=0; i<probes.size()-1; i++) cout<<probes[i]<<"->";
        cout<<probes[probes.size()-1]<<endl;
    }
        
};

// ---------------- CUSTOM PROBING ----------------
template<typename K,typename V>
class HashTableCustom{
    int C1,C2;
public:
    int size,nElements,lastExpansion,lastCompaction,collisionCount;
    vector<Entry<K,V>*> table;
    vector<bool> deleted;

    HashTableCustom(int c1,int c2,int s=INITIAL_SIZE): C1(c1), C2(c2) {
        size=s;nElements=0;
        collisionCount=0;
        table.resize(size,nullptr);
        deleted.resize(size,false);
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
        auto old=table; auto oldDel=deleted;
        table.clear(); deleted.clear();
        table.resize(newSize,nullptr); deleted.resize(newSize,false);
        size=newSize;nElements=0;
        

        auto polyWrapper = [](const string &s) -> size_t { return polyHash(s); };
        
        for(int i=0;i<(int)old.size();i++)
            if(old[i] && !oldDel[i]) insert(old[i]->key,old[i]->value,polyWrapper);
    }

    bool insert(const K &key,const V &value,function<size_t(const string&)> hashFunc){
        size_t h1=hashFunc(keyToString(key))%size;
        size_t h2=auxHash(key,size);
        int firstdel = -1;
        for(int i=0;i<size;i++){
            size_t idx=(h1+C1*i*h2+C2*i*i)%size;
            if(deleted[idx]){
                if(table[idx] && firstdel == -1) { 
                    firstdel = idx;
                }
            }
            if(!table[idx] && !deleted[idx]){
                if(firstdel != -1) idx = firstdel;
                table[idx]=new Entry<K,V>(key,value);
                deleted[idx]=false;
                nElements++;
                adjustSize();
                return true;
            } 
            else if(table[idx] && table[idx]->key==key) return false;
            collisionCount++;
        }
        return false;
    }

    V search(const K &key,int &hits,function<size_t(const string&)> hashFunc){
        size_t h1=hashFunc(keyToString(key))%size;
        size_t h2=auxHash(key,size);
        hits=0;
        for(int i=0;i<size;i++){
            size_t idx=(h1+C1*i*h2+C2*i*i)%size;
            hits++;
            if(!table[idx] && !deleted[idx]) return V();
            if(table[idx] && !deleted[idx] && table[idx]->key==key) return table[idx]->value;
        }
        return V();
    }

    bool remove(const K &key,function<size_t(const string&)> hashFunc){
        size_t h1=hashFunc(keyToString(key))%size;
        size_t h2=auxHash(key,size);
        for(int i=0;i<size;i++){
            size_t idx=(h1+C1*i*h2+C2*i*i)%size;
            if(!table[idx] && !deleted[idx]) return false;
            if(table[idx] && !deleted[idx] && table[idx]->key==key){
                deleted[idx]=true;
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
    mt19937 rng(time(0));
    int C1,C2; cin>>C1>>C2;

    int N=10000, wordLen=10, searchCount=10000;
    
    set<string> used;
    vector<string> words;

    // Generate unique words
    while((int)words.size()<N){
        string w=generateWord(wordLen,rng);
        if(used.count(w)) continue;
        used.insert(w);
        words.push_back(w);
    }
    //cout<<words.size()<<endl;
    shuffle(words.begin(),words.end(),rng); //c++ standard fnc to shuffle words randomly

    vector<pair<string,function<size_t(const string&)>>> hashFuncs = {
        {"poly", [](const string &s){ return polyHash(s); }},
        {"djb2", [](const string &s){ return djb2Hash(s); }}
    };

    cout<<"Technique\tHashFunc\tCollisions\tAvg Hits\n";

    for(auto &hf:hashFuncs){

        HashTableChaining<string,int> htC;
        HashTableDouble<string,int> htD;
        HashTableCustom<string,int> htP(C1,C2);
  
        int val = 1;
        for(auto &w:words){
            htC.insert(w,val,hf.second);
            htD.insert(w,val,hf.second);
            htP.insert(w,val,hf.second);
            val++;
        }

        int hitsC=0,hitsD=0,hitsP=0,tempHits;
        for(int i=0;i<searchCount;i++){
            htC.search(words[i],tempHits,hf.second); hitsC+=tempHits;
            htD.search(words[i],tempHits,hf.second); hitsD+=tempHits;
            htP.search(words[i],tempHits,hf.second); hitsP+=tempHits;
        }

        cout<<"Chaining\t"<<hf.first<<"\t\t"<<htC.collisionCount<<"\t\t"<<(double)hitsC/searchCount<<"\n";
        cout<<"Double\t\t"<<hf.first<<"\t\t"<<htD.collisionCount<<"\t\t"<<(double)hitsD/searchCount<<"\n";
        cout<<"Custom\t\t"<<hf.first<<"\t\t"<<htP.collisionCount<<"\t\t"<<(double)hitsP/searchCount<<"\n";


        //------------B online --------------
        // int n;
        // cin>>n;
        // int w;
        // for(int i=0; i<n; i++){
        //     cin>>w;
        //     htD.printProbeSequence(words[w], tempHits, hf.second);
        // }

    }

    return 0;
}