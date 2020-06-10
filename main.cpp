#include <iostream>
#include <vector>
#include <exception>
#include <ctime>
#include <sstream>
#include <fstream>
#include <string.h>

using namespace std;

//======================= [ GLOBAL VARIABLES ] =======================
char* file_name;
const int MEM_SIZE = 30000;

enum Operations {
    OPERATION_PLUS = 1,
    OPERATION_MINUS,
    OPERATION_RIGHT,
    OPERATION_LEFT,
    OPERATION_PRINT,
    OPERATION_LOOP
};

//======================= [ FUNCTIONS ] =======================

template<typename T>
std::string toString(const T &t) {
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

std::string time_hhmmss() {
    time_t times = time(0);
    auto now = localtime(&times);
    return (toString(now->tm_hour) + ":" + toString(now->tm_min) + ":" + toString(now->tm_sec));
}

//======================= [ INTERFACES ] =======================
class ICompiler {
    public:
    virtual void compile() = 0;
    virtual void run() = 0;
};

//======================= [ ERROR CODES ] =======================

#define E_ARGUMENTS_NUMBER 1
#define E_INCORRECT_FILENAME 2
#define E_MEMORY 3
#define E_CANNOT_READ_CODE 4
#define E_UNKNOWN_SYNTAX 5
#define E_WRONG_BRACKETS 6
#define E_INFINITE_LOOP 7
#define E_WRONG_MOVE 8

//======================= [ ERRORS ] =======================

class Error : public exception {
protected:
    int err_id;
    string msg;
    string param;

    string what() {
        return this->msg;
    }

    string prefix_time() {
        return "[" + time_hhmmss() + "]: ";
    }

public:
    Error() { this->err_id = 0; }

    Error(int err_id) {
        this->err_id = err_id;
        this->param = "";
        define_msg();
    }

    Error(int err_id, string param) {
        this->err_id = err_id;
        this->param = param;
        define_msg();
    }

    void define_msg() {
        switch(this->err_id) {
            case 1: {
                this->msg = "Wrong number of arguments";
                break;
            }
            case 2: {
                this->msg = "Incorrect name of file with source code";
                break;
            }
            case 3: {
                this->msg = "Memory error";
                break;
            }
            case 4: {
                this->msg = "Cannot read source code from this file";
                break;
            }
            case 5: {
                if(param.empty()) this->msg = "Syntax error: unknown symbol";
                else this->msg = ("Syntax error: unknown symbol \'" + this->param + "\'");
                break;
            }
            case 6: {
                this->msg = "Syntax error: wrong brackets";
                break;
            }
            case 7: {
                this->msg = "Runtime error: an infinite while loop";
                break;
            }
            case 8: {
                this->msg = "Wrong movement: out of range";
                break;
            }
            default: {
                this->msg = "Unknown error";
                break;
            }
        }
    }

    string get_error() {
        return (prefix_time() + "Error! " + what());
    }
};

//======================= [ CLASSES ] =======================

class Compiler : public ICompiler {
    private:
    static vector<pair<int, int>> data;
    static string result;

    string read_code(char* file_name) {                         //READING CODE FROM FILE.
        string code = "";
        char buf;
        ifstream in(toString(file_name));
        if(!in.is_open()) throw Error(E_INCORRECT_FILENAME);
        while(in.get(buf)) {
            code += toString(buf);
        }
        in.close();
        return code;
    }


    class Checker {                                             //CLASS FOR CHECKING CODE
        private:
        static string code;

        public:
        static const char allowed_syntax[8];
        static const char excesses[3];

        static void getCode(const string& code) {
            Checker::code = code;
        }

        static void remove_excesses() {
            for(size_t i = 0; i < code.length(); i++) {
                for(auto& excess : excesses) {
                    if(code.find(excess) != string::npos) code.erase(code.find(excess), 1);
                }
            }
        }

        static string check_syntax() {
            for(auto& item : code) {
                bool flag = false;
                for(auto& exist_symbol : allowed_syntax) {
                    if(item == exist_symbol) flag = true;
                }
                if(!flag) return toString(item);
            }
            return "";
        }

        static bool check_brackets() {
            int pairs_of_brackets = 0;
            for(auto& item : code) {
                if(item == '[') pairs_of_brackets++;
                else if(item == ']') pairs_of_brackets--;
            }
            if(pairs_of_brackets == 0) return true; else return false;
        }

        static void contract_instructions() {
            int loop_control = code.length();
            while(2 + 2 == 4) {
                bool is_found = false;
                for(size_t i = 0; i < code.length(); i++) {
                    if((code[i] == '+' && code[i+1] == '-') || (code[i] == '[' && code[i+1] == ']') || (code[i] == '>' && code[i+1] == '<') ) {
                        code.erase(i, 2);
                        is_found = true;
                        break;
                    }
                }
                if(!is_found) break;
                loop_control--;
                if(loop_control < 0) throw Error(E_INFINITE_LOOP);
            }
        }

        static vector<string> group() {                                         //SEPARATING OPERATIONS INTO GROUPS
            vector<string> temp;

            string tmp = toString(code[0]);
            //int begin_sym = 0;
            if(code.length() == 1) { temp.push_back(tmp); } else {
            //cout << code << endl;
            for(size_t i = 1; i < code.length(); i++) {
                if(code[i] == code[i-1]) tmp += code[i];
                else {
                    temp.push_back(tmp);
                    tmp = code[i];
                }
            }
            }
            return temp;
        }

        static void form(const vector<string>& temp) {                          //FORMING THE DATA TO RUN
            for(size_t i = 0; i < temp.size(); i++) {
                int* tmp = new int(0);
                if(temp[i][0] == '+') {
                    *tmp = (int)temp[i].length();
                    data.push_back(make_pair(OPERATION_PLUS, *tmp));
                }
                else if(temp[i][0] == '-') {
                    *tmp = (int)temp[i].length();
                    data.push_back(make_pair(OPERATION_MINUS, *tmp));
                }
                else if(temp[i][0] == '>') {
                    *tmp = (int)temp[i].length();
                    data.push_back(make_pair(OPERATION_RIGHT, *tmp));
                }
                else if(temp[i][0] == '<') {
                    *tmp = (int)temp[i].length();
                    data.push_back(make_pair(OPERATION_LEFT, *tmp));
                }
                else if(temp[i][0] == '.') {
                    *tmp = (int)temp[i].length();
                    data.push_back(make_pair(OPERATION_PRINT, *tmp));
                }
                else if(temp[i][0] == '[') {
                    *tmp = (int)temp[i].length();

                    int endLoop = i;
                    vector<string> tmp2;
                    for(size_t j = i + 1; j < temp.size(); j++) {
                        if(temp[j][0] == ']') { endLoop = j; break; }
                        else {
                            tmp2.push_back(temp[j]);
                        }
                    }

                    *tmp = -1;
                    data.push_back(make_pair(OPERATION_LOOP, *tmp));
                    form(tmp2);
                    *tmp = -2;
                    data.push_back(make_pair(OPERATION_LOOP, *tmp));

                    i = endLoop;
                }
                delete tmp;
            }
        }
    };

    class Builder {                                                             //CLASS FOR RUNNING CODE
        private:
        static char cpu[MEM_SIZE];
        static int pointer;
        static int minIndex;
        static int maxIndex;

        static void Add(int value) {
            cpu[pointer] += value;
        }

        static void Move(int value) {
            if(value > 0 && (pointer + value) > MEM_SIZE) throw Error(E_WRONG_MOVE);
            else if(value < 0 && (pointer + value) < 0) throw Error(E_WRONG_MOVE);
            pointer += value;
            if(pointer > maxIndex) maxIndex = pointer;
        }

        static void Print() {
            char tmp = cpu[pointer];
            string tmp2 = toString(tmp);
            result += tmp2;
        }

        static int Loop(int& index) {
            if(data[index].second == -1) {
                int beginLoop = index + 1;
                int endLoop = index + 1;
                for(size_t i = index; i < data.size(); i++) {
                    if(data[i].first == 6 && data[i].second == -2) {
                        endLoop = i;
                        break;
                    }
                }
                while((int)cpu[pointer] > 1) {
                    parse(beginLoop, endLoop);
                }
                return endLoop + 1;
            }
            else {
                return index + 1;
            }
        }

        static void giveResult() {
            cout << result << endl;
        }

        public:
        static void parse(int start, int finish) {                                  //PARSING DATA FOR RUNNING AND DOING OPERATIONS
            for(int i = start; i < finish; i++) {
                if(data[i].first == OPERATION_PLUS) Add(data[i].second);
                else if(data[i].first == OPERATION_MINUS) Add(-data[i].second);
                else if(data[i].first == OPERATION_RIGHT) Move(data[i].second);
                else if(data[i].first == OPERATION_LEFT) Move(-data[i].second);
                else if(data[i].first == OPERATION_PRINT) Print();
                else if(data[i].first == OPERATION_LOOP) Loop(i);
            }
            giveResult();                         //DISPLAYING A RESULT IF IT EXISTS
        }
    };

    public:
    void compile() override {
        if(file_name == "") throw Error();
        Checker::getCode(read_code(file_name));
        Checker::remove_excesses();
        if(!Checker::check_syntax().empty()) throw Error(E_UNKNOWN_SYNTAX, Checker::check_syntax());
        if(!Checker::check_brackets()) throw Error(E_WRONG_BRACKETS);
        Checker::contract_instructions();

        Checker::form(Checker::group());
    }

    void run() override {
        Builder::parse(0, data.size());
    }
};

//======================= [ INITIALIZING STATIC FIELDS ] =======================

string Compiler::Checker::code = "";
string Compiler::result = "";
vector<pair<int, int>> Compiler::data;
const char Compiler::Checker::allowed_syntax[8]{'<', '>', '+', '-', '.', '[', ']'};
const char Compiler::Checker::excesses[3]{' ', '\n', '\t'};

char Compiler::Builder::cpu[30000];
int Compiler::Builder::pointer = 0;
int Compiler::Builder::minIndex = 0;
int Compiler::Builder::maxIndex = 0;


//======================= [ COMMAND PATTERN ] =======================

class Command {
    public:
    virtual ~Command() {}
    virtual void execute() = 0;

    protected:
    Command(Compiler* p) : pcomp(p) { }
    Compiler* pcomp;
};

class CompileCommand : public Command {
    public:
    CompileCommand(Compiler* p) : Command(p) {}

    ~CompileCommand() { cout << "Compiling is finished" << endl; }

    void execute() {
        pcomp->compile();
    }
};

class RunCommand : public Command {
    public:
    RunCommand(Compiler* p) : Command(p) {}

    ~RunCommand() {
        cout << "Running is finished" << endl;
    }

    void execute() {
        pcomp->run();
    }
};

//======================= [ MAIN ] =======================

int main(int argc, char *argv[])
{
    try {
        if(argc != 2) throw Error(E_ARGUMENTS_NUMBER);

        file_name = argv[1];

        Compiler comp;
        vector<Command*> v;
        v.push_back(new CompileCommand(&comp));
        v.push_back(new RunCommand(&comp));

        for(size_t i = 0; i < v.size(); i++) {
            v[i]->execute();
        }

        for(size_t i = 0; i < v.size(); i++) {
            delete v[i];
        }
    }
    catch(Error& e) {
        cout << e.get_error() << endl;
    }
    return 0;
}
