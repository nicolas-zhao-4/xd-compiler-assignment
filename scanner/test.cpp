#include "scanner.h"

int main(int argc, char *argv[]){
    string f = argv[1];
    scanner scan(f);
    Token t;
    while((t=scan.getToken()).type != Token_Type::NONTOKEN){

        cout<<"main: "<<type(t.type)+"  "+t.text+"  "<<t.value<<endl;
    }
}

