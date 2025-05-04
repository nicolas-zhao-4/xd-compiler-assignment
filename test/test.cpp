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
#define red RGB(255,0,0)

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
    ERRTOKEN,                                                     //出错记号
    GUESS,
    IGN
};
                            
typedef struct Token{
    Token_Type type;                      //记号的种类
    string text;                          //构成记号的字符串
    double value;                         //若为常数，则是常数的值
    double(*FuncPtr)(double);             //若为函数，则是函数的指针
}Token;


static Token TokenTable[] = 				  //符号表内容 18
{
	{ Token_Type::CONST_ID,	"PI", 		3.1415926,		NULL },
	{ Token_Type::CONST_ID,	"E",		2.71828,		NULL },
	{ Token_Type::T,		"T",		0.0,			NULL },
	{ Token_Type::FUNC,		"SIN",		0.0,			sin },
	{ Token_Type::FUNC,		"COS",		0.0,			cos },
	{ Token_Type::FUNC,		"TAN",		0.0,			tan },
	{ Token_Type::FUNC,		"LN",		0.0,			log },
	{ Token_Type::FUNC,		"EXP",		0.0,			exp },
	{ Token_Type::FUNC,		"SQRT",		0.0,			sqrt },
	{ Token_Type::ORIGIN,	"ORIGIN",	0.0,			NULL },
	{ Token_Type::SCALE,	"SCALE",	0.0,			NULL },
	{ Token_Type::ROT,		"ROT",		0.0,			NULL },
	{ Token_Type::IS,		"IS",		0.0,			NULL },
	{ Token_Type::FOR,		"FOR",		0.0,			NULL },
	{ Token_Type::FROM,		"FROM",		0.0,			NULL },
	{ Token_Type::TO,		"TO",		0.0,			NULL },
	{ Token_Type::STEP,		"STEP",		0.0,			NULL },
	{ Token_Type::DRAW,		"DRAW",		0.0,			NULL },
};


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

string type(Token_Type t){
    if(t==Token_Type::ORIGIN){
        return "ORIGIN";
    }
    else if(t==Token_Type::SCALE){
        return "SCALE";
    }else if(t==Token_Type::ROT){
        return "ROT";
    }else if(t==Token_Type::IS){
        return "IS";
    }else if(t==Token_Type::TO){
        return "TO";
    }else if(t==Token_Type::STEP){
        return "STEP";
    }else if(t== Token_Type::DRAW){
        return " DRAW";
    }else if(t==Token_Type::FOR){
        return "FOR";
    }else if(t==Token_Type::FROM){
        return "FROM";
    }else if(t==Token_Type::T){
        return "T";
    }else if(t==Token_Type::SEMICO){
        return "SEMICO";
    }else if(t==Token_Type::L_BRACKET){
        return "L_BRACKET";
    }else if(t==Token_Type::R_BRACKET){
        return "R_BRACKET";
    }else if(t==Token_Type::COMMA){
        return "COMMA";
    }else if(t==Token_Type::PLUS){
        return "PLUS";
    }else if(t==Token_Type::MINUS){
        return "MINUS";
    }else if(t==Token_Type::MUL){
        return "MUL";
    }else if(t== Token_Type::DIV){
        return " DIV";
    }else if(t==Token_Type::POWER){
        return "POWER";
    }else if(t==Token_Type::FUNC){
        return "FUNC";
    }else if(t==Token_Type::CONST_ID){
        return "CONST_ID";
    }else if(t==Token_Type::NONTOKEN){
        return "NONTOKEN";
    }else if(t==Token_Type::ERRTOKEN){
        return "ERRTOKEN";
    }

    return "oops";
}

//数组序号为状态序号,终态表
//Token_Type::GUESS,不确定是T,还是CONST_ID,还是保留字  200,注释,忽略
const Token_Type DFA::final_states[14]={Token_Type::ERRTOKEN,Token_Type::GUESS,Token_Type::CONST_ID,Token_Type::CONST_ID,Token_Type::MUL,Token_Type::POWER,Token_Type::DIV,Token_Type::MINUS,Token_Type::PLUS,Token_Type::COMMA,Token_Type::SEMICO,Token_Type::L_BRACKET,Token_Type::R_BRACKET,Token_Type::IGN};

//状态转移储存
//字符一律看作A,数字一律看作1
const map<pair<int,char>,int> DFA::dfa={
    {{0,'*'},4},{{0,'/'},6},{{0,'-'},7},{{0,'+'},8},{{0,','},9},{{0,';'},10},{{0,'('},11},{{0,')'},12},
    {{6,'/'},13},{{7,'-'},13},{{0,'A'},1},{{1,'A'},1},{{0,'1'},2},{{2,'1'},2},{{2,'.'},3},{{3,'1'},3}
};

//返回TOKEN类型,非终态-1,Token_Type::GUESS不确定,200注释,确定可以简单确定的Token值:数字字面值,函数,保留字,PI,E
Token_Type DFA::finalState(int src,string buffer,Token& token){
    Token_Type state = DFA::final_states[src];
    if(state == Token_Type::CONST_ID){
        token.value=stod(buffer);
    }
    if(state==Token_Type::GUESS){
        for(int i=0;i<18;i++){
            if(buffer == TokenTable[i].text){
                state=TokenTable[i].type;
                token.value=TokenTable[i].value;
                token.FuncPtr = TokenTable[i].FuncPtr;
                break;
            }
        }
    }
    if(state==Token_Type::GUESS) return Token_Type::ERRTOKEN;//no match in simple table
    return state;
}

int DFA::move(int src,char tag){//if not exists in map ,map will throw exception
    int to=-1;
    try{
        if(isalpha(tag)) to=DFA::dfa.at(pair(src,'A'));
        else if(isdigit(tag)) to = DFA::dfa.at(pair(src,'1'));
        else to = DFA::dfa.at(pair(src,tag));
    }catch(exception e){
        //not exist
        return -1;
    }
    return to;
}

scanner::scanner(string fileName){
    file = make_unique<ifstream> (fileName);
    if(!file->is_open()){
        cout<<"[error] scanner:file open fail\n";
    }
    this->buffer="";
    this->buffer.reserve(30);
    this->lineNo = 1;
}

Token scanner::getToken(){
    buffer="";
    char t;
    tmp_token.value=0;
    tmp_token.FuncPtr=NULL;

    t=skip();
    if(t==EOF){
        tmp_token.type=Token_Type::NONTOKEN;
        return tmp_token;
    }

    int state = 0;
    int pre_state=0;
    while(true){
        buffer.append(1,t);
        pre_state=state;
        state = DFA::move(state,t);

        if(state==-1){                  //错了错了
            if(pre_state!=0){           //没错!
                file->unget();          //什么也没看见
                buffer = buffer.substr(0,buffer.size()-1);

                Token_Type s = DFA::finalState(pre_state,buffer,tmp_token);
                if(s==Token_Type::IGN) {//注释
                    getline(*file,buffer);//next line
                    lineNo++;
                    buffer="";
                    state=0;
                    pre_state=0;
                    t=skip();
                    continue;
                }
                if(s==Token_Type::ERRTOKEN){
                    err_msg(buffer);
                }
                tmp_token.type = s;
                tmp_token.text = buffer;
                break;
            }else{                      //真错了
                err_msg(buffer);
                tmp_token.type = Token_Type::ERRTOKEN;
                tmp_token.text = buffer;
                break;
            }
        }

        t=get();

        // if(t==' ' || t == '\n'){
        //     if(t=='\n') lineNo++;
        //     int s = DFA::finalState(state,buffer,tmp_token);
        //     if(s==200) {//注释
        //         getline(*file,buffer);//next line
        //         buffer="";
        //         state=0;
        //         pre_state=0;
        //         t=skip();
        //         continue;
        //     }
        //     if(s==ERRTOKEN){
        //         err_msg(buffer);
        //     }
        //     tmp_token.type = (Token_Type)s;
        //     tmp_token.text = buffer;
        //     break;
        // }
        
    }

    return tmp_token;

}

char scanner::skip(){//跳过space,\n
    char t=EOF;
    while(true){
        t=get();
        if(t==EOF){
            return t;
        }
        if(t=='\n'){
            this->lineNo++;
            continue;
        }
        if(!isspace(t)) break;
    }
    return t;
}

char scanner::get(){
    char c=file->get();
    c=toupper(c);
    return c;
}

void scanner::err_msg(string buffer){
    cout<<"\033[0;31m[ERROR]\033[0m SCANNER: error token: \'"+buffer+"\' in line:"<<lineNo<<"\n";
}

class ExprNode                                   //语法树expression节点类型
{
    public:
        enum Token_Type OpCode;                       //记号种类
        union Content
        {
            struct { shared_ptr<ExprNode> left, right; }CaseOperator;     //二元运算：只有左右孩子的内部节点
            struct { shared_ptr<ExprNode> Child; FuncPtr MathFuncPtr; }CaseFunc;//函数调用：只有一个孩子的内部节点，还有一个指向对应函数名的指针 MathFuncPtr
            double CaseConst;                                   //常数
            double *CaseParmPtr;                                //参数T
            Content(){              //因为含有shared_ptr,需要
                CaseParmPtr = NULL;
            }
            ~Content(){
                CaseParmPtr = NULL;
                CaseOperator.left.~shared_ptr();
                CaseOperator.right.~shared_ptr();
                CaseFunc.Child.~shared_ptr();
                CaseFunc.MathFuncPtr=NULL;
            }
            
        }Content;

        double getValue();

        void printTree();

        //
        ExprNode(enum Token_Type type,shared_ptr<ExprNode> left,shared_ptr<ExprNode> right){
            this->OpCode = type;
            this->Content.CaseOperator.left = left;
            this->Content.CaseOperator.right = right;
        }
        ExprNode(enum Token_Type type,shared_ptr<ExprNode> child,FuncPtr funcptr){
            this->OpCode = type;
            this->Content.CaseFunc.Child = child;
            this->Content.CaseFunc.MathFuncPtr = funcptr;
        }
        ExprNode(enum Token_Type type,double constid){
            this->OpCode = type;
            this->Content.CaseConst = constid;
        }
        ExprNode(enum Token_Type type,double* constid){
            this->OpCode = type;
            this->Content.CaseParmPtr = constid;
        }
        
};

void DrawPixel(unsigned long x, unsigned long y,HDC h){
	        SetPixel(h, x, y, red);
}

class parser{
    public:
        parser(string fileName,HDC h){
            Scanner = make_unique<scanner>(fileName);
            this->h=h;
            getToken();
            Program();
        }

    private:
        double Origin_x = 0, Origin_y = 0, Scale_x = 1, Scale_y = 1, Rot_angle = 0;
        double parameter,from,to,step,x,y;
        Token cur_token;
        unique_ptr<scanner> Scanner;
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
        void matchToken(enum Token_Type type){
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

double ExprNode::getValue(){
    switch(OpCode){
		case Token_Type::PLUS:
			return Content.CaseOperator.left->getValue() +
				Content.CaseOperator.right->getValue();
		case Token_Type::MINUS:
			return Content.CaseOperator.left->getValue() -
				Content.CaseOperator.right->getValue();
		case Token_Type::MUL:
			return Content.CaseOperator.left->getValue() *
				Content.CaseOperator.right->getValue();
		case Token_Type::DIV:
			return Content.CaseOperator.left->getValue() /
				Content.CaseOperator.right->getValue();
		case Token_Type::POWER:
			return pow(Content.CaseOperator.left->getValue(),
				Content.CaseOperator.right->getValue());
		case Token_Type::FUNC:
			return (Content.CaseFunc.MathFuncPtr)
				(Content.CaseFunc.Child->getValue());
		case Token_Type::CONST_ID:
			return Content.CaseConst;
		case Token_Type::T:
			return *(Content.CaseParmPtr);
		default:
			return 0.0;
	}
}

void ExprNode::printTree(){
    switch(OpCode){
        case Token_Type::FUNC:{
            Content.CaseFunc.Child->printTree();
            printf(" FUNC ");
            return;
            break;
        }
        case Token_Type::CONST_ID:{
            printf(" %lf ",Content.CaseConst);
            return;
            break;
        }
        case Token_Type::T:{
            printf(" T ");
            return;
            break;
        }
        default:{
            Content.CaseOperator.left->printTree();
            printf(" %s ",type(OpCode));
            Content.CaseOperator.right->printTree();
            return;
        }
    }
}

void parser::Program(){
    cout<<"enter Program\n";
    while(cur_token.type!=Token_Type::NONTOKEN){
        Statement();
        matchToken(Token_Type::SEMICO);
    }
    cout<<"leave Program\n";
}

void parser::Statement(){
    printf("enter statement\n");
    switch (cur_token.type)
	{
		//根据匹配到的保留字进入对应匹配语句函数
		case Token_Type::ORIGIN: OriginStatement(); break;
		case Token_Type::SCALE: ScaleStatement(); break;
		case Token_Type::ROT:  RotStatement(); break;
		case Token_Type::FOR: ForStatement(); break;
		default: error(3); 	//否则报错
	}
	printf("leave from Statement\n");
}

void parser::OriginStatement(){
    shared_ptr<ExprNode> tmp;
    printf("enter OriginStatement\n");
    matchToken(Token_Type::ORIGIN);
    matchToken(Token_Type::IS);
    matchToken(Token_Type::L_BRACKET);

    tmp = Expression();
    Origin_x = tmp->getValue();
    printf("tree: ");
    tmp->printTree();
    printf("\n");

    matchToken(Token_Type::COMMA);

    tmp = Expression();//此时上一个树失去所有shared_ptr,自动销毁
    Origin_y = tmp->getValue();
    printf("tree: ");
    tmp->printTree();
    printf("\n");

    matchToken(Token_Type::R_BRACKET);
    printf("leave Originstatement\n");
}

void parser::ScaleStatement(){
    shared_ptr<ExprNode> tmp;
    printf("enter ScaleStatement\n");

    matchToken(Token_Type::SCALE);
    matchToken(Token_Type::IS);
    matchToken(Token_Type::L_BRACKET);

    tmp = Expression();
    Scale_x = tmp->getValue();
    printf(" tree: ");
    tmp->printTree();
    printf("\n");

    matchToken(Token_Type::COMMA);

    tmp = Expression();
    Scale_y = tmp->getValue();
    printf(" tree: ");
    tmp->printTree();
    printf("\n");

    matchToken(Token_Type::R_BRACKET);
    printf("leave ScaleStatement\n");
}

void parser::RotStatement(){
    printf("enter RotStatement\n");
    shared_ptr<ExprNode> tmp;

    matchToken(Token_Type::ROT);
    matchToken(Token_Type::IS);

    tmp = Expression();
    Rot_angle = tmp->getValue();
    tmp->printTree();
    printf("\nleave RotStatement\n");
}

void parser::ForStatement(){
    printf("enter ForStatement\n");
    shared_ptr<ExprNode> tmp,left_ptr,right_ptr;

    matchToken(Token_Type::FOR);
    matchToken(Token_Type::T);
    matchToken(Token_Type::FROM);

    tmp = Expression();
    from = tmp->getValue();
    printf(" tree :");
    tmp->printTree();
    printf("\n");

    matchToken(Token_Type::TO);

    tmp = Expression();
    to = tmp->getValue();
    printf(" tree :");
    tmp->printTree();
    printf("\n");

    matchToken(Token_Type::STEP);

    tmp = Expression();
    step = tmp->getValue();
    printf("tree: ");
    tmp->printTree();
    printf("\n");

    matchToken(Token_Type::DRAW);
    matchToken(Token_Type::L_BRACKET);

    left_ptr = Expression();
    printf(" tree :");
    left_ptr->printTree();
    printf("\n");

    matchToken(Token_Type::COMMA);

    right_ptr = Expression();
    printf(" tree :");
    right_ptr->printTree();
    printf("\n");

    matchToken(Token_Type::R_BRACKET);

    drawLoop(left_ptr,right_ptr);

    printf("leavr ForStatement\n");
}

shared_ptr<ExprNode> parser::Expression(){
    printf("enter Expression\n");
    shared_ptr<ExprNode> left,right;
    enum Token_Type tem_type;

    left = Term();

    while(cur_token.type==Token_Type::MINUS || cur_token.type == Token_Type::PLUS){
        tem_type = cur_token.type;
        matchToken(cur_token.type);
        right = Term();
        left = make_shared<ExprNode> (tem_type,left,right);
    }

    printf("leave Expression\n");
    return left;
}

shared_ptr<ExprNode> parser::Term(){
    printf("enter Term\n");
    shared_ptr<ExprNode> left,right;
    enum Token_Type tem_type;

    left = Factor();

    while(cur_token.type==Token_Type::DIV || cur_token.type == Token_Type::MUL){
        tem_type = cur_token.type;
        matchToken(cur_token.type);
        right = Factor();
        left = make_shared<ExprNode> (tem_type,left,right);
    }

    printf("leave Term\n");
    return left;
}

shared_ptr<ExprNode> parser::Factor(){
    printf("enter Factor\n");
    shared_ptr<ExprNode> left,right;

    if(cur_token.type == Token_Type::PLUS){
        matchToken(Token_Type::PLUS);
        right = Factor();
    }else if(cur_token.type == Token_Type::MINUS){
        matchToken(Token_Type::MINUS);
        right = Factor();
        left = make_shared<ExprNode>(Token_Type::CONST_ID,0);
        right = make_shared<ExprNode>(Token_Type::MINUS,left,right);
    }else{
        right = Component();
    }

    printf("leave Factor\n");
    return right;
}

shared_ptr<ExprNode> parser::Component(){
    printf("enter Component\n");
    shared_ptr<ExprNode> left,right;

    left = Atom();
    if(cur_token.type == Token_Type::POWER){
        right = Component();
        left = make_shared<ExprNode>(Token_Type::POWER,left,right);
    }

    return left;
}

shared_ptr<ExprNode> parser::Atom(){
    printf("enter Atom\n");
    shared_ptr<ExprNode> left,right;

    switch(cur_token.type){
        case Token_Type::CONST_ID:{
            left = make_shared<ExprNode>(Token_Type::CONST_ID,cur_token.value);
            matchToken(Token_Type::CONST_ID);
            break;
        }
        case Token_Type::T:{
            left = make_shared<ExprNode>(Token_Type::T,&parameter);
            matchToken(Token_Type::T);
            break;
        }
        case Token_Type::FUNC:{
            Token tmp = cur_token;
            matchToken(Token_Type::FUNC);
            matchToken(Token_Type::L_BRACKET);

            right = Expression();
            matchToken(Token_Type::R_BRACKET);

            left = make_shared<ExprNode> (Token_Type::FUNC,right,tmp.FuncPtr);
            break;
        }
        case Token_Type::L_BRACKET:{
            matchToken(Token_Type::L_BRACKET);
            left = Expression();
            matchToken(Token_Type::R_BRACKET);
            break;
        }
        default:{
            error(4);
        }
    }
    return left;
    
}

void parser::drawLoop(shared_ptr<ExprNode> left,shared_ptr<ExprNode>right){
    for(parameter = from;parameter<=to;parameter+=step){
        cal_dot(left,right);
        DrawPixel(x,y,h);
    }
}

void parser::cal_dot(shared_ptr<ExprNode> left,shared_ptr<ExprNode> right){
    double tx,ty;
    x=left->getValue();
    y=right->getValue();//原始坐标
    x*=Scale_x;
    y*=Scale_y;         //比例变换
    tx=x;
    ty=y;               //旋转变换
    x=tx*cos(Rot_angle) + ty*sin(Rot_angle);
    y=ty*cos(Rot_angle) - tx*sin(Rot_angle);
    x+=Origin_x;
    y+=Origin_y;
}

void parser::error(int e){
    switch(e){
        case 1:{
            printf("parser: got errortoken\n");
            break;
        }
        case 2:{
            printf("parser: got unexpected token\n");
            break;
        }
        case 3:{
            printf("parser: got unexpected statement\n");
            break;
        }
        case 4:{
            printf("parser: got unexpected atom\n");
            break;
        }
        default:{
            printf("unknown error\n");
        }
    }
    exit(1);
}

const char g_szClassName[] = "myWindowClass";

char filename[50];
// Step 4: the Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    //Step 1: Registering the Window Class
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0,0,0));
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Step 2: Creating the Window
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "The title of my window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 690,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    HDC hDC = GetDC(hwnd);
    string str;
    strcpy(filename,lpCmdLine);
    str = filename;

    parser p(str,hDC);


    UpdateWindow(hwnd);

    // Step 3: The Message Loop
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}