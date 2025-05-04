#include "parser.h"

void DrawPixel(unsigned long x, unsigned long y,HDC h){
	SetPixel(h, (unsigned long)x,(unsigned long) y, red);
}

ExprNode::ExprNode(Token_Type type,shared_ptr<ExprNode> left,shared_ptr<ExprNode> right){
                this->OpCode = type;
                this->Content.CaseOperator.left = left;
                this->Content.CaseOperator.right = right;
}


ExprNode::ExprNode(Token_Type type,shared_ptr<ExprNode> child,FuncPtr funcptr){
            this->OpCode = type;
            this->Content.CaseFunc.Child = child;
            this->Content.CaseFunc.MathFuncPtr = funcptr;
}

ExprNode::ExprNode(Token_Type type,double constid){
            this->OpCode = type;
            this->Content.CaseConst = constid;
}

ExprNode::ExprNode(Token_Type type,double* constid){
            this->OpCode = type;
            this->Content.CaseParmPtr = constid;
}

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
            if(Content.CaseFunc.Child == NULL)
                cout<<"func have null child\n";
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
            if(Content.CaseOperator.left == NULL)
                cout<<"optree left null\n";
            Content.CaseOperator.left->printTree();
            cout<<" "<<type(OpCode);
            if(Content.CaseOperator.right == NULL)
                cout<<"optree right null\n";
            Content.CaseOperator.right->printTree();
            return;
        }
    }
}

parser::parser(string fileName,HDC h){
            Scanner = make_unique<scanner>(fileName);
            debugFlie = ofstream("debug.txt");
            cout<<"in parser\n";
            if(!debugFlie.is_open())
                cout<<"debug file open fail\n";
            this->h=h;
            getToken();
            Program();
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
    // printf("enter OriginStatement\n");
    matchToken(Token_Type::ORIGIN);
    matchToken(Token_Type::IS);
    matchToken(Token_Type::L_BRACKET);

    tmp = Expression();
    Origin_x = tmp->getValue();
    printf("tree: ");
    tmp->printTree();
    cout<<endl<<"origin_x: "<<Origin_x<<endl;
    tmp.reset();
    matchToken(Token_Type::COMMA);

    tmp = Expression();//此时上一个树失去所有shared_ptr,自动销毁
    Origin_y = tmp->getValue();
    printf("tree: ");
    tmp->printTree();
    cout<<endl<<"origin_y: "<<Origin_y<<endl;
    printf("\n");
    tmp.reset();
    matchToken(Token_Type::R_BRACKET);
    // printf("leave Originstatement\n");
}

void parser::ScaleStatement(){
    shared_ptr<ExprNode> tmp;
    // printf("enter ScaleStatement\n");

    matchToken(Token_Type::SCALE);
    matchToken(Token_Type::IS);
    matchToken(Token_Type::L_BRACKET);

    tmp = Expression();
    Scale_x = tmp->getValue();
    printf(" tree: ");
    tmp->printTree();
    cout<<endl<<"scale_x: "<<Scale_x<<endl;
    printf("\n");
    tmp.reset();
    matchToken(Token_Type::COMMA);

    tmp = Expression();
    Scale_y = tmp->getValue();
    printf(" tree: ");
    tmp->printTree();
    cout<<endl<<"scale_y: "<<Scale_y<<endl;
    printf("\n");
    tmp.reset();
    matchToken(Token_Type::R_BRACKET);
    // printf("leave ScaleStatement\n");
}

void parser::RotStatement(){
    // printf("enter RotStatement\n");
    shared_ptr<ExprNode> tmp;

    matchToken(Token_Type::ROT);
    matchToken(Token_Type::IS);

    tmp = Expression();
    Rot_angle = tmp->getValue();
    printf(" tree: ");
    tmp->printTree();
    cout<<endl<<"ROT_angle: "<<Rot_angle<<endl;
    // printf("\nleave RotStatement\n");
}

void parser::ForStatement(){
    // printf("enter ForStatement\n");
    shared_ptr<ExprNode> tmp,left_ptr,right_ptr;

    matchToken(Token_Type::FOR);
    matchToken(Token_Type::T);
    matchToken(Token_Type::FROM);

    tmp = Expression();
    from = tmp->getValue();
    printf(" tree :");
    tmp->printTree();
    cout<<endl<<"from: "<<from<<endl;
    printf("\n");
    tmp.reset();

    matchToken(Token_Type::TO);

    tmp = Expression();
    to = tmp->getValue();
    printf(" tree :");
    tmp->printTree();
    cout<<endl<<"to: "<<to<<endl;
    printf("\n");
    tmp.reset();
    matchToken(Token_Type::STEP);

    tmp = Expression();
    step = tmp->getValue();
    printf("tree: ");
    tmp->printTree();
    cout<<endl<<"step: "<<step<<endl;
    printf("\n");
    tmp.reset();
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

    // printf("leave ForStatement\n");
}

shared_ptr<ExprNode> parser::Expression(){
    printf("enter Expression\n");
    shared_ptr<ExprNode> left,right;
    Token_Type tem_type;

    left = Term();

    while(cur_token.type==Token_Type::MINUS || cur_token.type == Token_Type::PLUS){
        tem_type = cur_token.type;
        matchToken(tem_type);
        right = Term();
        left = make_shared<ExprNode> (tem_type,left,right);
    }

    // cout<<"expression get tree: ";
    // left->printTree();
    // cout<<endl;

    printf("leave Expression\n");


    return left;
}

shared_ptr<ExprNode> parser::Term(){
    printf("enter Term\n");
    shared_ptr<ExprNode> left=nullptr,right=nullptr,tmp_ptr=nullptr;
    Token_Type tem_type;

    left = Factor();

    while(cur_token.type==Token_Type::DIV || cur_token.type == Token_Type::MUL){
        tem_type = cur_token.type;
        matchToken(tem_type);
        right = Factor();
        left = make_shared<ExprNode> (tem_type,left,right);
    }

    printf("leave Term\n");
    // cout<<"term get tree: ";
    // left->printTree();
    // cout<<endl;
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
    // cout<<"factor get tree: ";
    // right->printTree();
    // cout<<endl;
    return right;
}

shared_ptr<ExprNode> parser::Component(){
    printf("enter Component\n");
    shared_ptr<ExprNode> left,right;

    left = Atom();
    if(cur_token.type == Token_Type::POWER){
        matchToken(Token_Type::POWER);
        right = Component();
        left = make_shared<ExprNode>(Token_Type::POWER,left,right);
    }
    printf("leave Component\n");

    // cout<<"component get tree: ";
    // left->printTree();
    // cout<<endl;
    return left;
}

shared_ptr<ExprNode> parser::Atom(){
    printf("enter Atom\n");
    shared_ptr<ExprNode> left=nullptr,right=nullptr;

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
    printf("leave Atom\n");
    // cout<<"atom get tree: ";
    // left->printTree();
    // cout<<endl;
    return left;
    
}


void parser::drawLoop(shared_ptr<ExprNode> left,shared_ptr<ExprNode>right){
    debugFlie<<"Loop: "<<from<<","<<to<<" step "<<step<<" origin:"<<Origin_x<<","<<Origin_y<<" scale: "<<Scale_x<<","<<Scale_y<<" rot: "<<Rot_angle<<endl;
    for(parameter = from;parameter<=to;parameter+=step){
        cal_dot(left,right);
        // debugFlie<<"x: "<<x<<" y: "<<y<<endl;
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
            printf("\033[0;31m[ERROR]\033[0m parser: got errortoken (error id %d)\n",e);
            break;
        }
        case 2:{
            printf("\033[0;31m[ERROR]\033[0m parser: got unexpected token (error id %d)\n",e);
            break;
        }
        case 3:{
            printf("\033[0;31m[ERROR]\033[0m parser: got unexpected statement (error id %d)\n",e);
            break;
        }
        case 4:{
            printf("\033[0;31m[ERROR]\033[0m parser: got unexpected atom (error id %d)\n",e);
            break;
        }
        default:{
            printf("\033[0;31m[ERROR]\033[0m unknown error (error id unknown)\n");
        }
    }
    exit(1);
}