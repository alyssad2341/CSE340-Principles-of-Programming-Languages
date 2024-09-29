/*
Author: Raneem Youssef, Alyssa Duranovic
Project 1 CSE 340 Spring 2024
Date: January 30, 2024
*/

/*
 * Copyright (C) Rida Bazzi 
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "inputbuf.h"

using namespace std; 

//different types of tokens
string reserved[] = { "END_OF_FILE",
    "IF", "WHILE", "DO", "THEN", "PRINT",
    "PLUS", "MINUS", "DIV", "MULT",
    "EQUAL", "COLON", "COMMA", "SEMICOLON",
    "LBRAC", "RBRAC", "LPAREN", "RPAREN",
    "NOTEQUAL", "GREATER", "LESS", "LTEQ", "GTEQ",
    "DOT", "NUM", "ID", "ERROR", "REALNUM", "BASE08NUM", "BASE16NUM" 
    // TODO: Add labels for new token types here (as string)
};

//key word tokens
#define KEYWORDS_COUNT 5
string keyword[] = { "IF", "WHILE", "DO", "THEN", "PRINT" };

//prints token information to the screen, including token name and line number
void Token::Print()
{
    cout << "{" << this->lexeme << " , "
         << reserved[(int) this->token_type] << " , "
         << this->line_no << "}\n";
}

//constructor for lexical analyzer
LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

//skips whitespace and keeps track of line number
bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    // increments line number if next char is a new line
    input.GetChar(c);
    line_no += (c == '\n');

    //after space is encountered, keeps reading until a non white space character or the end of the input
    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }

    //if there's still more input, unget the most recent char
    if (!input.EndOfInput()) {
        input.UngetChar(c);
    }
    return space_encountered; //return whether or not white space is encountered
}

//checks if a string is a keyword
bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

//finds the index of the keyword and returns the token type
TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR; //if the string is not a keyword
}

//scans for number tokens
Token LexicalAnalyzer::ScanNumber()
{
    char c;
    std:string temp; //string used in case the value has letters but isnt a base16 number

    input.GetChar(c);
    if (isdigit(c)) {
        //if the digit starts with a 0
        if (c == '0') {
            tmp.lexeme = "0";
        } else {
            tmp.lexeme = "";
            //keep adding digits if its a number
            while (!input.EndOfInput() && isdigit(c)) {
                tmp.lexeme += c;
                input.GetChar(c);
            }
            //otherwise unget the char and try to see if the number is a decimal, base 8, or base 16 
            if (!input.EndOfInput()) {
                input.UngetChar(c);
            }
        }

        //checks for decimal
        input.GetChar(c);
        if(c == '.') {
            //get next digit
            input.GetChar(c);
            //if its a valid decimal
            if (isdigit(c)) {
                tmp.lexeme += '.'; //add the decimal and traverse until you run out of digits
                while (!input.EndOfInput() && isdigit(c)) {
                    tmp.lexeme += c;
                    input.GetChar(c);
                }

                //unget the last char if its not a valid number
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }

                //return the real number token
                tmp.token_type = REALNUM;
                tmp.line_no = line_no;
                return tmp;
            }

            //if invalid decimal, return the num token 
            else {
                input.UngetChar(c); //unget the char after the decimal
                input.UngetChar('.'); //unget the decimal
                tmp.token_type = NUM;
                tmp.line_no = line_no;
                return tmp;
                
            }

        }

        //potential base 16 numbers
        if ((c == 'A') || (c == 'B') || (c == 'C') || (c == 'D') || (c == 'E') || (c == 'F')) {
            while (!input.EndOfInput() && (isdigit(c) || (c == 'A') || (c == 'B') || (c == 'C') || (c == 'D') || (c == 'E') || (c == 'F'))) {
                    temp += c; //add to the temp string
                    input.GetChar(c);
            }
        }


        //could be base 08 or base 16
        if(c == 'x') {
            input.GetChar(c);
            //potentially a base 08
            if (c == '0') {
                input.GetChar(c);
                if(c == '8') { //it is a base 08 number
                    input.UngetString("x08"); //unget the base 8 part
                    input.GetChar(c); 

                    while (isdigit(c)) { //keep consuming characters until you reach the x
                        tmp.lexeme += c;
                        input.GetChar(c);
                    }

                    //consume the 08 part and return a base08 token
                    input.GetChar(c);
                    input.GetChar(c);
                    tmp.lexeme += "x08";
                    tmp.token_type = BASE08NUM;
                    tmp.line_no = line_no;
                    return tmp;

                }
                
                //if its x0 and then some other number
                else {
                    if (!input.EndOfInput()) {
                        input.UngetChar(c);
                    }
                    input.UngetString("x0");
                    input.UngetString(temp);

                } 

            }

            //potentially a base 16
            else if(c == '1') {
                input.GetChar(c);
                if(c == '6') { //it is a base 16 number
                    input.UngetString("x16");//unget the base 16 part
                    input.GetChar(c); 

                    //update lexeme with the earlier values now that we know it is base 16
                    tmp.lexeme += temp;
                    while (c != 'x') { //keep consuming characters until you reach the x
                        tmp.lexeme += c;
                        input.GetChar(c);
                    }

                    //consume the 16 part and return a base 16 token
                    input.GetChar(c);
                    input.GetChar(c);
                    tmp.lexeme += "x16";
                    tmp.token_type = BASE16NUM;
                    tmp.line_no = line_no;
                    return tmp; 

                }

                //if its x1 and then some other number
                else {
                    if (!input.EndOfInput()) {
                        input.UngetChar(c);
                    }
                    input.UngetString("x1");
                    input.UngetString(temp);

                }

            }

            //if its just a lone x, unget both characters
            else {
                input.UngetChar(c);
                input.UngetChar('x');
                input.UngetString(temp);
            }

        }

        //if there is no x
        else {
            input.UngetChar(c);
            input.UngetString(temp);
            
        }

        //default is return a Num token
        tmp.token_type = NUM;
        tmp.line_no = line_no;
        return tmp;

    // error token
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
        tmp.line_no = line_no;
        return tmp;
    }
}

//identify ID or keywords
Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    input.GetChar(c);

    //checks if c is a letter of the alphabet (bc id's start with a letter)
    if (isalpha(c)) {
        tmp.lexeme = "";
        //continue as long as input is an alphabet or number
        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c; //add character to string
            input.GetChar(c);
        }

        //if it's not the end of the input, unget the last character
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }

        tmp.line_no = line_no;
        //if the token is a key word, find the index, otherwise, the token is an ID
        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    //if the string doesn't start with a letter of the alphabet, it's an error
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}


//unget the token and add it back to the input buffer
TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

//returns the token type from input
Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) { //if there are some characters in tokens
        tmp = tokens.back(); //pop last element in vector
        tokens.pop_back();
        return tmp; // return last token element
    }

    SkipSpace();
    tmp.lexeme = ""; //empty lexeme string
    tmp.line_no = line_no;
    input.GetChar(c);
    //check the value of c and assign corresponding token type to the tmp.token_type variable
    switch (c) {
        case '.':
            tmp.token_type = DOT;
            return tmp;
        case '+':
            tmp.token_type = PLUS;
            return tmp;
        case '-':
            tmp.token_type = MINUS;
            return tmp;
        case '/':
            tmp.token_type = DIV;
            return tmp;
        case '*':
            tmp.token_type = MULT;
            return tmp;
        case '=':
            tmp.token_type = EQUAL;
            return tmp;
        case ':':
            tmp.token_type = COLON;
            return tmp;
        case ',':
            tmp.token_type = COMMA;
            return tmp;
        case ';':
            tmp.token_type = SEMICOLON;
            return tmp;
        case '[':
            tmp.token_type = LBRAC;
            return tmp;
        case ']':
            tmp.token_type = RBRAC;
            return tmp;
        case '(':
            tmp.token_type = LPAREN;
            return tmp;
        case ')':
            tmp.token_type = RPAREN;
            return tmp;
        case '<':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = LTEQ;
            } else if (c == '>') {
                tmp.token_type = NOTEQUAL;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = LESS;
            }
            return tmp;
        case '>':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = GTEQ;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = GREATER;
            }
            return tmp;
        default:
        
            if (isdigit(c)) {
                input.UngetChar(c);
                return ScanNumber();
            } else if (isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrKeyword();
            } else if (input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else
                tmp.token_type = ERROR;

            return tmp;
    }
}

//bool LexicalAnalyzer::

int main()
{
    LexicalAnalyzer lexer;
    Token token;

    token = lexer.GetToken();
    token.Print();
    while (token.token_type != END_OF_FILE)
    {
        token = lexer.GetToken();
        token.Print();
    }
}
