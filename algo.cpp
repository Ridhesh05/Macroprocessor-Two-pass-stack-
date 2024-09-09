#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stack>

using namespace std;

const string INPUT_FILE1 = "input3.txt";
const string INPUT_FILE2 = "input2.txt";
const string MNT_FILE = "mnt.txt";
const string MDT_FILE = "mdt.txt";
const string OUTPUT_FILE = "output.txt";

map<string, int> MNT;   
vector<string> MDT;     
map<string, string> ALA; 
stack<pair<string, int>> callStack;

vector<string> split(const string &line, char delimiter = ' ') {
    vector<string> tokens;
    string token;
    for (char ch : line) {
        if (ch == delimiter && !token.empty()) {
            tokens.push_back(token);
            token.clear();
        } else if (ch != delimiter) {
            token += ch;
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

void pass1(const vector<string> &inputLines) {
    bool inMacro = false;
    string macroName;

    for (const string &line : inputLines) {
        vector<string> tokens = split(line);

        if (tokens.empty()) continue;

        if (tokens[0] == "MACRO") {
            inMacro = true;
            continue;
        }

        if (inMacro && tokens[0] != "MEND") {
            if (macroName.empty()) {
                macroName = tokens[0];
                MNT[macroName] = MDT.size();
            }
            MDT.push_back(line);
        } else if (tokens[0] == "MEND") {
            MDT.push_back(line);
            inMacro = false;
            macroName.clear();
        }
    }

    ofstream mntFile(MNT_FILE);
    ofstream mdtFile(MDT_FILE);

    if (!mntFile.is_open() || !mdtFile.is_open()) {
        cerr << "Error opening MNT or MDT files!" << endl;
        return;
    }

    for (const auto &mntEntry : MNT) {
        mntFile << mntEntry.first << " " << mntEntry.second << endl;
    }

    for (size_t i = 0; i < MDT.size(); ++i) {
        mdtFile << i << ": " << MDT[i] << endl;
    }
}

void pass2(const vector<string> &inputLines) {
    ofstream outputFile(OUTPUT_FILE);
    vector<string> outputLines;

    auto expandMacro = [&](const string &macroName, const vector<string> &actualArgs) {
        int MDTpointer = MNT[macroName];
        ALA.clear();

        string firstLine = MDT[MDTpointer];
        vector<string> dummyArgs = split(firstLine.substr(firstLine.find(' ') + 1));

        for (size_t i = 0; i < dummyArgs.size(); ++i) {
            ALA[dummyArgs[i]] = actualArgs[i];
        }

        MDTpointer++;

        while (MDTpointer < MDT.size() && MDT[MDTpointer].find("MEND") == string::npos) {
            string expandedLine = MDT[MDTpointer];

            for (const auto &entry : ALA) {
                size_t pos = expandedLine.find(entry.first);
                while (pos != string::npos) {
                    expandedLine.replace(pos, entry.first.length(), entry.second);
                    pos = expandedLine.find(entry.first, pos + entry.second.length());
                }
            }

            outputLines.push_back(expandedLine);
            MDTpointer++;
        }
    };

    for (const string &line : inputLines) {
        vector<string> tokens = split(line);

        if (MNT.find(tokens[0]) != MNT.end()) {
            vector<string> actualArgs(tokens.begin() + 1, tokens.end());
            callStack.push({tokens[0], MNT[tokens[0]]});
            expandMacro(tokens[0], actualArgs);
            callStack.pop();
        } else {
            outputLines.push_back(line);
        }
    }

    for (const string &outputLine : outputLines) {
        outputFile << outputLine << endl;
    }
}

int main() {
    vector<string> inputLines1, inputLines2;
    string line;

    ifstream inputFile1(INPUT_FILE1);
    while (getline(inputFile1, line)) {
        inputLines1.push_back(line);
    }

    ifstream inputFile2(INPUT_FILE2);
    while (getline(inputFile2, line)) {
        inputLines2.push_back(line);
    }

    vector<string> combinedInput = inputLines1;
    combinedInput.insert(combinedInput.end(), inputLines2.begin(), inputLines2.end());

    pass1(combinedInput);
    pass2(combinedInput);

    cout << "Macro processing completed. Check 'mnt_table.txt', 'mdt_table.txt', and 'output.txt' for results." << endl;

    return 0;
}
