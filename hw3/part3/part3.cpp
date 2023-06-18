#include "part3.h"

int main(int argc, char* argv[]){
    if(argc <3){ // we have 4 type parameter but some of dont have parameter
        cerr<<"Number of paramter is wrong."<<endl;
        cerr<<"Usage : ./fileSystemOper fileSystem.data operation parameters"<<endl;
        exit(EXIT_FAILURE);
    }
    std::filesystem::path programPath = argv[0]; // ı take error from recognize argv
    string programname=programPath.filename().string();
    if(programname != "fileSystemOper"){ // cmp example
        cerr<<"argv[0] is wrong. "<<endl;
        exit(EXIT_FAILURE);
    }
    Parameter par(argv[1]);
    par.setS(argc, argv);
    par.process();

    cout<<endl<<endl;
}