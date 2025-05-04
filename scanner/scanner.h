#pragma once
#ifndef _SCANNER_H
#define _SCANNER_H
#include <iostream>
#include <string>
#include <math.h>
#include <fstream>
#include <memory>
#include <map>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

typedef double (*FuncPtr)(double);

enum class Token_Type                                                  //记号种类
{
    ORIGIN, SCALE, ROT, IS, TO,STEP, DRAW, FOR, FROM,            //保留字
    T,                                                           //参数
    SEMICO, L_BRACKET, R_BRACKET, COMMA,                         //分隔符号
    PLUS, MINUS, MUL, DIV, POWER,                                //运算符
    FUNC,                                                        //函数
    CONST_ID,                                                    //常数
    NONTOKEN,                                                    //空记号
    ERRTOKEN,                                                    //出错记号
    GUESS,
    IGN
};
                            
typedef struct Token{
    Token_Type type;                      //记号的种类
    string text;                          //构成记号的字符串
    double value;                         //若为常数，则是常数的值
    double(*FuncPtr)(double);             //若为函数，则是函数的指针
}Token;


extern Token TokenTable[];


class scanner{
    
    public:
        scanner(string filenName);
        scanner(char* fileName){
            scanner((string)fileName);
        }
        ~scanner(){
            file->close();
        }
        Token getToken();
        int cur_line(){
            return lineNo;
        }
    private:
        unique_ptr<ifstream> file;      //智能指针,离开作用域自动回收内存
        string buffer;                  //缓冲区
        int lineNo;                     //当前行数
        Token tmp_token;                //准备返回的token
        char get();                     //下一个char,转大写
                                        //file->unget(),回退
        char skip();                    //跳过space \n 返回EOF与char    
        void err_msg(string buffer);
        bool is_open(){
            return file->is_open();
        }                          
};

class DFA{
    public:
        static int move(int src,char tag);
        static Token_Type finalState(int src,string buffer,Token& token);
    private:
        static const map<pair<int,char>,int> dfa;
        static const Token_Type final_states[14];
};

string type(Token_Type t);

#endif