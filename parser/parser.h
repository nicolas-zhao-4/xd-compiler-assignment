#ifndef _PARSER_H
#define _PARSER_H
#include "../scanner/scanner.h"



#define red RGB(255,0,0)
#define black RGB(0,0,0)


class ExprNode                                   //语法树expression节点类型
{
    public:
        Token_Type OpCode;                       //记号种类
        /*这里卡了我好久好久(少说5h),进行了各种调试,结果每次都是不一样的错误,本来这里是union content,然后如果这里是union,那么当我通过构造函数传参(shared_ptr<ExprNode>类型)
        过来后,推测会导致shared_ptr 机制识别不到union的shared_ptr<ExprNode>,因为他不知道这个union里头到底是存放了什么类型,也有可能是赋值进union
        不会增加shared_ptr 的引用数量,于是,这片空间就这样被释放掉了....之前的分析直接白搭,于是出现神奇的一幕,有时没有问题,再来一遍,欸,又有问题了
        */
        struct Content
        {
            struct { shared_ptr<ExprNode> left, right; }CaseOperator;     //二元运算：只有左右孩子的内部节点
            struct { shared_ptr<ExprNode> Child; FuncPtr MathFuncPtr; }CaseFunc;//函数调用：只有一个孩子的内部节点，还有一个指向对应函数名的指针 MathFuncPtr
            double CaseConst;                                   //常数
            double *CaseParmPtr;                                //参数T      
        }Content;

        double getValue();

        void printTree();

        //
        ExprNode(Token_Type type,shared_ptr<ExprNode> left,shared_ptr<ExprNode> right);
        ExprNode(Token_Type type,shared_ptr<ExprNode> child,FuncPtr funcptr);
        ExprNode(Token_Type type,double constid);
        ExprNode(Token_Type type,double* constid);
        
};

void DrawPixel(unsigned long x, unsigned long y,HDC h);

class parser{
    public:
        parser(string fileName,HDC h);
        ~parser(){
            debugFlie.close();
        }
    private:
        double Origin_x = 0, Origin_y = 0, Scale_x = 1, Scale_y = 1, Rot_angle = 0;
        double parameter,from,to,step,x,y;
        Token cur_token;
        unique_ptr<scanner> Scanner;
        ofstream debugFlie;
        HDC h;
        void Program();         	//程序
        void Statement();		    //语句
        void OriginStatement();	    //Origin语句
        void RotStatement();		//Rot语句
        void ScaleStatement();	    //Scale语句
        void ForStatement();		//For语句

        shared_ptr<ExprNode> Expression();	//表达式、二元加减运算表达式
        shared_ptr<ExprNode> Term();			//乘除运算表达式
        shared_ptr<ExprNode> Factor();		//一元加减运算表达式
        shared_ptr<ExprNode> Component();	//幂运算表达式
        shared_ptr<ExprNode> Atom();			//原子表达式

        void getToken(){
            cur_token=Scanner->getToken();
            if(cur_token.type == Token_Type::ERRTOKEN){
                error(1);
            }
        }
        void matchToken(Token_Type type){
            if(cur_token.type != type){
                error(2);
            }
            getToken();
        }
        void error(int id);


        //things about draw
        void cal_dot(shared_ptr<ExprNode> left,shared_ptr<ExprNode> right);
        void drawLoop(shared_ptr<ExprNode> left,shared_ptr<ExprNode> right);
};



#endif