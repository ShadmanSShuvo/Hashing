// 1. using unordered_map

#include <iostream>
#include <unordered_map>
#include <regex>

using namespace std;

// Function to print unique words in a string
void printUniqueWords(string str)
{
    // Extracting words from string
    regex pattern("[a-zA-Z]+");
    smatch match;

    // Map to store count of a word
    unordered_map<string, int> wordCount;

    // Iterating over words in the string
    while (regex_search(str, match, pattern))
    {
        string word = match.str();

        // If this is the first occurrence of the word
        if (wordCount.find(word) == wordCount.end())
            wordCount[word] = 1;
        else
            // Increment counter of the word
            wordCount[word] += 1;

        // Move to the next match
        str = match.suffix();
    }

    // Traverse map and print all words whose count is 1
    for (auto const &pair : wordCount)
    {
        if (pair.second == 1)
            cout << pair.first << endl;
    }
}

// Driver Method
int main()
{
    string str = "Java is great. Grails is also great";
    printUniqueWords(str);
    return 0;
}



// 2. using set
// Function to print unique words
/* 
#include <iostream>
#include <vector>
#include <unordered_set>
#include <sstream>

// Function to print unique words
void printWords(const std::vector<std::string>& words) {
    std::unordered_set<std::string> uniqueWords;

    // For loop for iterating
    for (const std::string& word : words) {
        // Check if the word has been printed before
        if (uniqueWords.find(word) == uniqueWords.end()) {
            std::cout << word << std::endl;
            uniqueWords.insert(word);
        }
    }
}

int main() {
    // Input string
    std::string str = "geeks for geeks";

    // Storing string in the form of a vector of words
    std::vector<std::string> wordVector;
    std::istringstream iss(str);
    std::string word;
    while (iss >> word) {
        wordVector.push_back(word);
    }

    // Passing vector to print words function
    printWords(wordVector);

    return 0;
}
*/