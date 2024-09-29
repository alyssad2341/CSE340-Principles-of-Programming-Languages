//Author: Raneem Youssef and Alyssa Duranovic

/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *               Rida Bazzi 2019
 * Do not share this file with anyone
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "lexer.h"
#include <set>
#include <map>
#include <algorithm>

using namespace std;
LexicalAnalyzer lexer;

vector<string> temp; //all non-terminals on first pass

vector<string> terminals;
vector<string> nonTerminals;

//grammar rule has a non terminal on the LHS and a list of strings on the RHS
struct grammarRule {
    string LHS;
    vector<string> RHS;
};

//vector to store all the rules
vector<grammarRule> rules;

//method declarations
void parseGrammar();
void parseRuleList();
void parseIdList();
void parseRule();
void parseRHS();
void syntax_error();
Token expect(Token);
bool isNonTerminal(const string&);

grammarRule rule;

// read grammar calls parse Grammar
void ReadGrammar()
{
    parseGrammar();
}

// syntax error function
void syntax_error()
{
	// outputs error message and exits the program
	cout << "SYNTAX ERROR !!!\n";
	exit(0);
}

// function that returns a token if it matches the expected type
Token expect(TokenType expected)
{
	Token t = lexer.GetToken();
	if (t.token_type != expected)
	{
		cout << "expect syntax";
		syntax_error();

	}
	return t;
}

//parse Grammar → Rule-list HASH
void parseGrammar() {
    parseRuleList();
    expect(HASH); 
}

//Rule → ID ARROW Right-hand-side STAR
void parseRule() {
    Token t = lexer.peek(1);
    //make sure there are no reprtitions and add all non-terminals
    if(temp.empty() || !isNonTerminal(t.lexeme)) {
        temp.push_back(t.lexeme);
    }
    
    if (t.token_type == HASH)
	{
		return;
	}
    
    rule.LHS = t.lexeme;
    expect(ID);
    expect(ARROW);
    parseRHS();
    expect(STAR);
    rules.push_back(rule);
    rule.RHS.clear();
}

//Id-list → ID Id-list | ID
void parseIdList() {
    Token t = lexer.peek(1);
	// if the next tokentype is an ID
	if (t.token_type == ID)
	{
        rule.RHS.push_back(t.lexeme);
        expect(ID);
        parseIdList();
		return;
	}
    
    // second definition
    else if (t.token_type == STAR) {
        return;
    }
    else {
        // syntax error
		syntax_error();
    }
    
}

//Rule-list → Rule Rule-list | Rule
void parseRuleList(){
    parseRule();
    Token t = lexer.peek(1);
    // If the next token type is ARROW, there is another Rule
    if (t.token_type == ID) {
        parseRule();
        parseRuleList();
    } else if (t.token_type == HASH) {
        return;
    }
    else {
        syntax_error();
    }
    
}

//parse RHS
void parseRHS() {
    Token t = lexer.peek(1);

    // If the next token type is an ID
    if (t.token_type == ID) {
        parseIdList();
        return;
    } else if (t.token_type == STAR) {
        return;
    }
    else {
        syntax_error();
    }
}

//check if a string is a non-terminal
bool isNonTerminal(const string& symbol) {
    //check against temp vector
    for (const string& nonTerminal : temp) {
        if (nonTerminal == symbol) {
            return true;  // Found a match, it's a non-terminal
        }
    }
    return false;  // not a non-terminal
}



// Task 1: prints terminals and non-terminals in the order they appear
void printTerminalsAndNoneTerminals()
{
    // Collect unique terminals
    set<string> uniqueTerminals;
    set<string> uniqueNonTerminals;
    vector<string> unique;

    // iterate through rules and add terminals and non terminals to their respective sets
    for (const auto& rule : rules) {
        uniqueNonTerminals.insert(rule.LHS); //add all LHS to non terminal set
        for (const auto& symbol : rule.RHS) {
            if (!isNonTerminal(symbol)) {
                uniqueTerminals.insert(symbol);
            } else {
                uniqueNonTerminals.insert(symbol);
            }
        }
    }

    //print terminals in order they were found and then erase from set
    for (const auto& rule : rules) {
        for (const auto& symbol : rule.RHS) {
            if (uniqueTerminals.find(symbol) != uniqueTerminals.end()) {
                cout << symbol << " ";
                uniqueTerminals.erase(symbol);  
            }
        }
    }

    //print nonterminals in the order they were found and erase from set
    for (const auto& rule : rules) {
        if (uniqueNonTerminals.find(rule.LHS) != uniqueNonTerminals.end()) {
                cout << rule.LHS << " ";
                uniqueNonTerminals.erase(rule.LHS);  // Remove the printed terminal
        }
        for (const auto& symbol : rule.RHS) {
            if (uniqueNonTerminals.find(symbol) != uniqueNonTerminals.end()) {
                cout << symbol << " ";
                uniqueNonTerminals.erase(symbol);  // Remove the printed terminal
            }
        }
    }

    cout << endl;
}

void GeneratingSymbols() {
    
    map<string, set<string>> symbolMap;
    //count how many unique terminals and nonterminals in rules
    int numTerminals = 0;
    int numNonTerminals = 0;
    int size = 0;

    // Collect unique terminals
    set<string> uniqueTerminals;
    set<string> uniqueNonTerminals;

    // Iterate through rules and add non-terminals to uniqueTerminals
    for (const auto& rule : rules) {
        uniqueNonTerminals.insert(rule.LHS);
        for (const auto& symbol : rule.RHS) {
            if (!isNonTerminal(symbol)) {
                uniqueTerminals.insert(symbol);
            }else {
                uniqueNonTerminals.insert(symbol);
            }
        }
    }

    //symbolMap stores whether symbol is generating or not
    //# is generating, $ is nongenerating
    symbolMap["#"].insert("t");
    symbolMap["$"].insert("f");

    //all terminals are generating
    for (const std::string& terminal : uniqueTerminals) {
        symbolMap[terminal].insert("t");
    }

    //initialize nonterminals as nongenerating
    for (const std::string& terminal : uniqueNonTerminals) {
        symbolMap[terminal].insert("f");
    }

    bool changed = true;
    bool generated = true;

    //if symbolMap changes, run through each of the rules again
    while(changed){
        changed = false;

        //if RHS is generating, then LHS is generating
        for (const auto& rule : rules) {
            generated = true;

            auto ti = symbolMap.find(rule.LHS);
 

            if (ti != symbolMap.end() && ti->second.count("f") > 0) {
                for (const auto& symbol : rule.RHS) {
                    auto it = symbolMap.find(symbol);

                    if (it != symbolMap.end()) {
                        if (it->second.count("f") > 0) {
                            
                            generated = false;
                        }
                    }
                }
                // Update the value associated with rule.LHS outside the inner loop
                if(generated){
                    ti->second.erase("f");
                    ti->second.insert("t");
                    changed = true;
                }

            }
    
        }
    }

    //remove all rules that are not generating
    vector<grammarRule> newRules;

    bool generatingRule = true;

    const std::string& firstNonterminal = rules[0].LHS;

    auto start = symbolMap.find(firstNonterminal);

    if (start->second.count("t") > 0){

        for (const auto& rule : rules) {
            generatingRule = true;
            
            auto ti = symbolMap.find(rule.LHS);

            if (ti != symbolMap.end() && ti->second.count("t") > 0) { 

                for (const auto& symbol : rule.RHS) {
                    auto it = symbolMap.find(symbol);

                        if (it != symbolMap.end()) {
                            if (it->second.count("f") > 0) {
                                generatingRule = false;
                            }
                        }
                }
            }else{
                generatingRule = false;
            }
            if(generatingRule){
                newRules.push_back(rule);
            }
            
        }

    }

    rules = newRules;

    
}


void ReachableSymbols() {

    map<string, set<string>> symbolMap2;

    // Collect unique terminals
    set<string> uniqueTerminals;
    set<string> uniqueNonTerminals;

    // Iterate through rules and add non-terminals to uniqueTerminals
    for (const auto& rule : rules) {
        uniqueNonTerminals.insert(rule.LHS); //add all LHS to non terminal set
        for (const auto& symbol : rule.RHS) {
            if (!isNonTerminal(symbol)) {
                uniqueTerminals.insert(symbol);
            }else {
                uniqueNonTerminals.insert(symbol);
            }
        }
    }


    //first nonTerminal in rules is reachable
    const std::string& firstNonterminal = rules[0].LHS;
    symbolMap2[firstNonterminal].insert("t");

    //initialize other nonTerminals as nonreachable
    for (const std::string& terminal : uniqueNonTerminals) {
        if(terminal != firstNonterminal){
            symbolMap2[terminal].insert("f");
        }
    }

    //initialize all terminals as nonreachable
    for (const std::string& terminal : uniqueTerminals) {
        if(terminal != firstNonterminal){
            symbolMap2[terminal].insert("f");
        }
    }

    bool changed = true;


    //if symbolMap is changed, run through all the rules again
    while(changed){
        changed = false;

        //if RHS is reachable, then LHS is reachable
        for (const auto& rule : rules) {

            auto ti = symbolMap2.find(rule.LHS);

            if(ti != symbolMap2.end() && ti->second.count("t") > 0){
                for (const auto& symbol : rule.RHS) {
                    auto it = symbolMap2.find(symbol);
                    if (it != symbolMap2.end() && it->second.count("f") > 0) {
                        it->second.erase("f");
                        it->second.insert("t");
                        changed = false;
                    }
                }
            }
    
        }
    }

    //remove rules that are nonreachable
    vector<grammarRule> newRules;

    bool reachableRule = true;

    for (const auto& rule : rules) {
        reachableRule = true;
        
        auto ti = symbolMap2.find(rule.LHS);

        if (ti != symbolMap2.end() && ti->second.count("t") > 0) { 

            for (const auto& symbol : rule.RHS) {
                auto it = symbolMap2.find(symbol);

                    if (it != symbolMap2.end()) {
                        if (it->second.count("f") > 0) {
                            reachableRule = false;
                        }
                    }
            }
        }else{
            reachableRule = false;
        }
        if(reachableRule){
            newRules.push_back(rule);
        }
        
    }

    rules = newRules;

    //print out the rules
    for (const auto& rule : rules) {
        std::cout << rule.LHS << " -> ";

        if (rule.RHS.empty()) {
            std::cout << "#";
        } else {
            for (const auto& symbol : rule.RHS) {
                std::cout << symbol << " ";
            }
        }

        std::cout << std::endl;
    }

}

// Task 2
void RemoveUselessSymbols()
{ 
    GeneratingSymbols();
    ReachableSymbols();
}

//outside method because will be used for follow sets as well
map<string, set<string>> firstSets;
// Task 3 to calculate first sets
void CalculateFirstSets()
{
    int oldsize = 0;
    int i = 0;

    //sets and vectors containing terminals and non-terminals
    set<string> uniqueTerminals;
    set<string> uniqueNonTerminals;
    vector<string> termVector;
    vector<string> nonTermVector;

    // iterate through rules and add terminals and non terminals to their respective sets
    for (const auto& rule : rules) {
        uniqueNonTerminals.insert(rule.LHS); //add all LHS to non terminal set
        for (const auto& symbol : rule.RHS) {
            if (!isNonTerminal(symbol)) {
                uniqueTerminals.insert(symbol);
            } else {
                uniqueNonTerminals.insert(symbol);
            }
        }
    }

    //add all terminals to terminal vector, used for printing
    for (const auto& rule : rules) {
        for (const auto& symbol : rule.RHS) {
            if (uniqueTerminals.find(symbol) != uniqueTerminals.end()) {
                termVector.push_back(symbol);
                uniqueTerminals.erase(symbol); 
            }
        }
    }

    //add all nonterminals to nonterminal vector, used for printing
    for (const auto& rule : rules) {
        //check LHS of rule
        if (uniqueNonTerminals.find(rule.LHS) != uniqueNonTerminals.end()) {
                nonTermVector.push_back(rule.LHS);
                uniqueNonTerminals.erase(rule.LHS);  
        }
        //then check RHS of rule
        for (const auto& symbol : rule.RHS) {
            if (uniqueNonTerminals.find(symbol) != uniqueNonTerminals.end()) {
                nonTermVector.push_back(symbol);
                uniqueNonTerminals.erase(symbol);  
            }
        }

    }

    //create all first sets for terminals and non terminals
    for (const auto& rule : rules) {
        string nonTerminal = rule.LHS;
        const vector<string>& rhs = rule.RHS;

        // check if the terminal has a first set 
        if (firstSets.find(nonTerminal) == firstSets.end()) {
            firstSets[nonTerminal] = set<string>(); 
        }

        //add epsilon to sets with empty rhs
        if (rhs.empty()) {
            firstSets[nonTerminal].insert("#");
        }

        //create first sets for all terminals and add the terminal to the first set
        for (const auto& symbol : rule.RHS) {
            if (!isNonTerminal(symbol) && firstSets.find(symbol) == firstSets.end()) {
                firstSets[symbol] = set<string>();  
                string temp = (string)symbol;
                firstSets[symbol].insert(temp);
            }
        }
    }

    //declare and instantiate variables
    int j = 0;
    bool changed = true;
    oldsize = 0;

    //keep going as long as the sets are changing
    while(changed) {
        oldsize = firstSets.size();
        for (const auto& rule : rules) {
            string nonTerminal = rule.LHS;
            const vector<string>& rhs = rule.RHS;

            //if the rhs is empty, move onto next rule
            if(rhs.size()== 0)  {
                continue;
            }

            //iterate through the rhs
            for (int i = 0; i < rhs.size(); i++) {
                string current = rhs[i];
                //epsilon in set, add first set - epsilon to the nonterminal first set
                if (firstSets[current].find("#") != firstSets[current].end()) {
                    firstSets[current].erase("#");
                    firstSets[nonTerminal].insert(firstSets[current].begin(), firstSets[current].end());  
                    firstSets[current].insert("#");
                    //if epsilons in first set of every term, epsilon in nonterminal first set
                    if (i == rhs.size() - 1) {
                        firstSets[nonTerminal].insert("#");
                    }
                }
                //no epsilon in set, add first set and break
                else {
                    firstSets[nonTerminal].insert(firstSets[current].begin(), firstSets[current].end());  
                    break;
                }
            }

        }

        //if the set stays the same size, increment a variable
        if (oldsize == firstSets.size()) {
            j++;
            //set must stay the same size for the number of nonterminals
            if (j == nonTermVector.size()) {
                changed = false;
                break;
            }
        }
    }

    // print the first sets
    for (const auto& nonTerminal : nonTermVector) {
        if (nonTerminal.empty()) {
            break;
        }
        cout << "FIRST(" << nonTerminal << ") = { ";

        //if there is an epsilon in the first set, print epsilon
        set<string> firstSet = firstSets[nonTerminal];
        if(firstSet.find("#") != firstSet.end()) {
            cout << "#";
            firstSet.erase("#");
            if (firstSet.size() != 0) {
                cout << ", ";
            }
        }

        //print terminals in the order they were found
        for (const auto& terminal : termVector) {
            if(firstSet.find(terminal) != firstSet.end()) {
                cout << terminal;
                firstSet.erase(terminal);
                if (firstSet.size() != 0) {
                    cout << ", ";
                }
            }
        }

        cout << " }\n";
    }
}



// Task 4: calculate follow set for all non terminals
void CalculateFollowSets()
{
    // Map for follow sets and set/vector declarations
    map<string, set<string>> followSets;
    set<string> uniqueTerminals;
    set<string> uniqueNonTerminals;
    vector<string> termVector;
    vector<string> nonTermVector;

    // iterate through rules and add terminals and non terminals to their respective sets
    for (const auto& rule : rules) {
        uniqueNonTerminals.insert(rule.LHS); //add all LHS to non terminal set
        for (const auto& symbol : rule.RHS) {
            if (!isNonTerminal(symbol)) {
                uniqueTerminals.insert(symbol);
            } else {
                uniqueNonTerminals.insert(symbol);
            }
        }
    }

    //add all terminals to terminal vector, used for printing
    for (const auto& rule : rules) {
        for (const auto& symbol : rule.RHS) {
            if (uniqueTerminals.find(symbol) != uniqueTerminals.end()) {
                termVector.push_back(symbol);
                uniqueTerminals.erase(symbol); 
            }
        }
    }

    //add all nonterminals to nonterminal vector, used for printing
    for (const auto& rule : rules) {
        //check LHS of rule
        if (uniqueNonTerminals.find(rule.LHS) != uniqueNonTerminals.end()) {
                nonTermVector.push_back(rule.LHS);
                uniqueNonTerminals.erase(rule.LHS);  
        }
        //then check RHS of rule
        for (const auto& symbol : rule.RHS) {
            if (uniqueNonTerminals.find(symbol) != uniqueNonTerminals.end()) {
                nonTermVector.push_back(symbol);
                uniqueNonTerminals.erase(symbol);  
            }
        }

    }

    //create all first sets for terminals and non terminals
    for (const auto& rule : rules) {
        string nonTerminal = rule.LHS;
        const vector<string>& rhs = rule.RHS;

        // check if the terminal has a first set 
        if (firstSets.find(nonTerminal) == firstSets.end()) {
            firstSets[nonTerminal] = set<string>(); 
        }

        //add epsilon to sets with empty rhs
        if (rhs.empty()) {
            firstSets[nonTerminal].insert("#");
        }

        //create first sets for all terminals and add the terminal to the first set
        for (const auto& symbol : rule.RHS) {
            if (!isNonTerminal(symbol) && firstSets.find(symbol) == firstSets.end()) {
                firstSets[symbol] = set<string>();  
                string temp = (string)symbol;
                firstSets[symbol].insert(temp);
            }
        }
    }

    //variable declarations
    int k = 0;
    bool change = true;
    int oldsize = 0;

    //loop to apply rules 3, 4, 5 of first sets
    while(change) {
        oldsize = firstSets.size();
        for (const auto& rule : rules) {
            string nonTerminal = rule.LHS;
            const vector<string>& rhs = rule.RHS;

            //if the rhs is empty, move onto next rule
            if(rhs.size()== 0)  {
                continue;
            }

            //iterate through the rhs
            for (int i = 0; i < rhs.size(); i++) {
                string current = rhs[i];
                //epsilon in set, add first set - epsilon to the nonterminal first set
                if (firstSets[current].find("#") != firstSets[current].end()) {
                    firstSets[current].erase("#");
                    firstSets[nonTerminal].insert(firstSets[current].begin(), firstSets[current].end());  
                    firstSets[current].insert("#");
                    //if epsilons in first set of every term, epsilon in nonterminal first set
                    if (i == rhs.size() - 1) {
                        firstSets[nonTerminal].insert("#");
                    }
                }
                //no epsilon in set, add first set and break
                else {
                    firstSets[nonTerminal].insert(firstSets[current].begin(), firstSets[current].end());  
                    break;
                }
            }

        }

        //if the set stays the same size, increment a variable
        if (oldsize == firstSets.size()) {
            k++;
            //set must stay the same size for the number of nonterminals
            if (k == nonTermVector.size()) {
                change = false;
                break;
            }
        }
    }
    
    //initilaize all follow sets to empty
    for (const auto& rule : rules) {
        string nonTerminal = rule.LHS;
        const vector<string>& rhs = rule.RHS;

        // if nonterminal doesnt have follow set, make one
        if (followSets.find(nonTerminal) == followSets.end()) {
            followSets[nonTerminal] = set<string>();  
        }
    }

    // Rule 1: follow set of starting variable must have $
    followSets[nonTermVector.front()].insert("$");

    
    //apply rules 4 and 5
    //iterate through rules
    for (const auto& rule : rules) {
        string nonTerminal = rule.LHS;
        const vector<string>& rhs = rule.RHS;

        string previous = "";

        //apply rule 4 starting from the second to last term
        for (int i = rhs.size() - 2; i >= 0; i--) {
            previous = rhs[i+1];
            string current = rhs[i];

            //if the current index is a nonterminal
            if (isNonTerminal(current)) {
                //if epsilon is in the first set
                if (firstSets[previous].find("#") != firstSets[previous].end()) {
                    firstSets[previous].erase("#");
                    followSets[current].insert(firstSets[previous].begin(), firstSets[previous].end());  
                    firstSets[previous].insert("#");
                }
                else {
                    followSets[current].insert(firstSets[previous].begin(), firstSets[previous].end());  
                }
            }
        }
        
        //apply rule 5
        for (int i = 0; i < rhs.size(); i++) {
            string current = rhs[i];

            // if the current symbol is a non-terminal
            if (isNonTerminal(current)) {
                // if epsilon in the first set
                if (firstSets[current].find("#") != firstSets[current].end()) {
                    // Iterate over the rest of the rhs
                    for (int l = i + 1; l < rhs.size(); l++) {
                        string next = rhs[l];

                        // If next symbol is a non-terminal
                        if (isNonTerminal(next)) {
                            //epsilon in set, add first set - epsilon to the follow set
                            if (firstSets[next].find("#") != firstSets[next].end()) {
                                firstSets[next].erase("#");
                                followSets[current].insert(firstSets[next].begin(), firstSets[next].end());
                                firstSets[next].insert("#");

                            }

                            // no epsilon, exit loop
                            else {
                                followSets[current].insert(firstSets[next].begin(), firstSets[next].end());
                                break;
                            }
                        // if next symbol is terminal, add to follow set
                        } else {
                            followSets[current].insert(next);
                            break;
                        }
                    }
                }
            }
        }
    }

    //declare variables
    int j = 0;
    bool changed = true;
    oldsize = 0;

    //loop to apply rules 2 and 3
    while(changed) {
        oldsize = followSets.size();
        for (const auto& rule : rules) {
            string nonTerminal = rule.LHS;
            const vector<string>& rhs = rule.RHS;

            //if the rhs is empty, break
            if(rhs.size()== 0)  {
                continue;
            }

            //combined rules 2 and 3, iterate from last element of rhs
            for (int i = rhs.size() - 1; i >= 0; i--) {
                string current = rhs[i];

                //exit loop if nonterminal
                if(!isNonTerminal(current)) {
                    break;
                }

                //epsilon is part of the first set of last and the last is a non terminal, add to follow set
                else if(firstSets[current].find("#") != firstSets[current].end()) {
                    if (isNonTerminal(current)) {
                        followSets[current].insert(followSets[nonTerminal].begin(), followSets[nonTerminal].end());  
                    }
                }

                //epsilon isn't part of first set but the current value is a nonTerminal
                else {
                    if (isNonTerminal(current)) {
                        followSets[current].insert(followSets[nonTerminal].begin(), followSets[nonTerminal].end());  
                        break;
                    }
                }
            }

        }

        //if no change in size for as many iterations as there are nonvectors, exit the loop
        if (oldsize == followSets.size()) {
            j++;
            if (j == nonTermVector.size()) {
                changed = false;
                break;
            }
        }
    }
    


    // Print follow sets
    for (const auto& nonTerminal : nonTermVector) {
        if (nonTerminal.empty()) {
            break;
        }
        cout << "FOLLOW(" << nonTerminal << ") = { ";

        set<string> followSet = followSets[nonTerminal];
        
        // if the follow set has $, print that first
        if(followSet.find("$") != followSet.end()) {
            cout << "$";
            followSet.erase("$");
            if (followSet.size() != 0) {
                cout << ", ";
            }
        }

        //check if each terminal is in the follow set and print in the order they were found
        for (const auto& terminal : termVector) {
            if(followSet.find(terminal) != followSet.end()) {
                cout << terminal;
                followSet.erase(terminal);
                if (followSet.size() != 0) {
                    cout << ", ";
                }
            }
        }

        cout << " }\n";
    }
}

    

//check if all the symbols are generating
bool allGeneratingSymbols() {
    //count how many unique terminals and nonterminals in rules
    int numTerminals = 0;
    int numNonTerminals = 0;
    int size = 0;

    map<string, set<string>> symbolMap3;

    bool allGenerating = true;

    // Collect unique terminals
    set<string> uniqueTerminals;
    set<string> uniqueNonTerminals;

    // Iterate through rules and add non-terminals to uniqueTerminals
    for (const auto& rule : rules) {
        uniqueNonTerminals.insert(rule.LHS); //add all LHS to non terminal set
        for (const auto& symbol : rule.RHS) {
            if (!isNonTerminal(symbol)) {
                uniqueTerminals.insert(symbol);
            }else {
                uniqueNonTerminals.insert(symbol);
            }
        }
    }


    symbolMap3["#"].insert("t");
    symbolMap3["$"].insert("f");

    for (const std::string& terminal : uniqueTerminals) {
        symbolMap3[terminal].insert("t");
    }

    for (const std::string& terminal : uniqueNonTerminals) {
        symbolMap3[terminal].insert("f");
    }

    bool changed = true;
    bool generated = true;


    while(changed){
        changed = false;
        for (const auto& rule : rules) {
            generated = true;

            auto ti = symbolMap3.find(rule.LHS);
 

            if (ti != symbolMap3.end() && ti->second.count("f") > 0) {
                for (const auto& symbol : rule.RHS) {
                    auto it = symbolMap3.find(symbol);

                    if (it != symbolMap3.end()) {
                        if (it->second.count("f") > 0) {
                            
                            generated = false;
                        }
                    }
                }
                // Update the value associated with rule.LHS outside the inner loop
                if(generated){
                    ti->second.erase("f");
                    ti->second.insert("t");
                    changed = true;
                }

            }
    
        }

    }

    vector<grammarRule> newRules;

    bool generatingRule = true;

    const std::string& firstNonterminal = rules[0].LHS;

    auto start = symbolMap3.find(firstNonterminal);

    if (start->second.count("t") > 0){

        for (const auto& rule : rules) {
            generatingRule = true;
            
            auto ti = symbolMap3.find(rule.LHS);

            if (ti != symbolMap3.end() && ti->second.count("t") > 0) { 

                for (const auto& symbol : rule.RHS) {
                    auto it = symbolMap3.find(symbol);

                        if (it != symbolMap3.end()) {
                            if (it->second.count("f") > 0) {
                                generatingRule = false;
                                allGenerating = false;
                            }
                        }
                }
            }else{
                generatingRule = false;
            }
            if(generatingRule){
                newRules.push_back(rule);
            }
            
        }

    }else{
        allGenerating = false;
    }

    rules = newRules;
    return allGenerating;

    
}


//check if all the symbols are reachable
bool allReachableSymbols() {

    map<string, set<string>> symbolMap4;

    bool allReachable = true;

    // Collect unique terminals
    set<string> uniqueTerminals;
    set<string> uniqueNonTerminals;

    // Iterate through rules and add non-terminals to uniqueTerminals
    for (const auto& rule : rules) {
        uniqueNonTerminals.insert(rule.LHS); //add all LHS to non terminal set
        for (const auto& symbol : rule.RHS) {
            if (!isNonTerminal(symbol)) {
                uniqueTerminals.insert(symbol);
            }else {
                uniqueNonTerminals.insert(symbol);
            }
        }
    }

    const std::string& firstNonterminal = rules[0].LHS;
    symbolMap4[firstNonterminal].insert("t");

    for (const std::string& terminal : uniqueNonTerminals) {
        if(terminal != firstNonterminal){
            symbolMap4[terminal].insert("f");
        }
    }
    for (const std::string& terminal : uniqueTerminals) {
        if(terminal != firstNonterminal){
            symbolMap4[terminal].insert("f");
        }
    }


    bool changed = true;

    while(changed){
        changed = false;
        for (const auto& rule : rules) {

            auto ti = symbolMap4.find(rule.LHS);

            if(ti != symbolMap4.end() && ti->second.count("t") > 0){
                for (const auto& symbol : rule.RHS) {
                    auto it = symbolMap4.find(symbol);
                    if (it != symbolMap4.end() && it->second.count("f") > 0) {
                        it->second.erase("f");
                        it->second.insert("t");
                        changed = false;
                    }
                }
            }

    
        }

    }

    vector<grammarRule> newRules;

    bool reachableRule = true;

    for (const auto& rule : rules) {
        reachableRule = true;
        
        auto ti = symbolMap4.find(rule.LHS);

        if (ti != symbolMap4.end() && ti->second.count("t") > 0) { 

            for (const auto& symbol : rule.RHS) {
                auto it = symbolMap4.find(symbol);

                    if (it != symbolMap4.end()) {
                        if (it->second.count("f") > 0) {
                            reachableRule = false;
                            allReachable = false;
                        }
                    }
            }
        }else{
            reachableRule = false;
            allReachable = false;
        }
        if(reachableRule){
            newRules.push_back(rule);
        }
        
    }

    rules = newRules;


    return allReachable;

}

// Task 5
void CheckIfGrammarHasPredictiveParser()
{
    
    if(allGeneratingSymbols() == true){
            if(allReachableSymbols() == true){
                std::cout << "YES\n";
            }else{
                std::cout << "NO\n";
            }
        }else{
            std::cout << "NO\n";
        }



}
    
int main (int argc, char* argv[])
{
    int task;

    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);
    
    ReadGrammar();  // Reads the input grammar from standard input
                    // and represent it internally in data structures
                    // ad described in project 2 presentation file

    switch (task) {
        case 1: printTerminalsAndNoneTerminals();
            break;

        case 2: RemoveUselessSymbols();
            break;

        case 3: CalculateFirstSets();
            break;

        case 4: CalculateFollowSets();
            break;

        case 5: CheckIfGrammarHasPredictiveParser();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}

