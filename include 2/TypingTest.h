#ifndef TYPINGTEST_H
#define TYPINGTEST_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>

class TypingTest {
public:
    TypingTest(const std::string& filePath) {
        std::srand(std::time(nullptr));
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filePath);
        }
        std::string word;
        while (file >> word) {
            words.push_back(word);
        }
    }

    std::string getRandomSentence(size_t wordCount) {
        std::ostringstream sentence;
        for (size_t i = 0; i < wordCount; ++i) {
            int index = std::rand() % words.size();
            sentence << words[index];
            if (i < wordCount - 1) {
                sentence << " ";
            }
        }
        return sentence.str();
    }

private:
    std::vector<std::string> words;
};

#endif // TYPINGTEST_H
