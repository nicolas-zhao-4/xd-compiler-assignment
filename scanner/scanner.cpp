#include "scanner.h"

Token TokenTable[] = 				  //符号表内容 18
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
        if(isalpha(tag)) to=DFA::dfa.at(pair<int,char>(src,'A'));
        else if(isdigit(tag)) to = DFA::dfa.at(pair<int,char>(src,'1'));
        else to = DFA::dfa.at(pair<int,char>(src,tag));
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