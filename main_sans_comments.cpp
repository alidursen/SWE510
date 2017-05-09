#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <stack>
#include <cstring>
#include <algorithm>

using namespace std;

/* ASCII LIST:
	* \n: 10
	* ' ': 32
	* (: 40
	* ): 41
		* *: 42
		* +: 43
		* -: 45
		* /: 47
		* =: 61
	* 0-9: 48-57
	* A-Z: 65-90
	* a-z: 97-122
* */

bool isAlph(char &c){ return ((c>64)&&(c<91)) || ((c>96)&&(c<123)); }
bool isNum(string &s){ return (s.find_first_of("()*+-/=")==string::npos); }

stack<int> OKU(string &input){ //int >=0 -> int, <0 char //accepts lines
	char c;
	bool fEq=false, fAdd=false, fMult=false;
	int p_counter=0, inl=input.length();
	stack<int> rslt, temp;
	string subs1="", subs2="";
		
	if( (inl==1 && isAlph(input.front())) || isNum(input) ){
		if(isAlph(input.front())){ rslt.push(-1*(input.front())); }
		else rslt.push(stoi(input));
		return rslt;
	}
	
	if(input[1]=='='){
		fEq = true;
		subs1 += input[0];
		c = '=';
		subs2 = input;
		subs2.erase(0,2);
	}
	
	if(!(fEq)){
		for(int i=0; i<inl; i++){
			c = input[i];
			if(c=='('){ p_counter++; }
			if(c==')'){ p_counter--; }
			if((p_counter==0)&&(c=='+' || c=='-')){ 
				fAdd = true;
				subs1.insert(0, input, 0, i);
				subs2.insert(0, input, i+1, string::npos);
				break;
			}
		}
	}
	
	if(!(fEq || fAdd)){
		for(int i=0; i<inl; i++){
			c = input[i];
			if(c=='('){ p_counter++; }
			if(c==')'){ p_counter--; }
			if((p_counter==0)&&(c=='*' || c=='/')){
				fMult = true;
				subs1.insert(0, input, 0, i);
				subs2.insert(0, input, i+1, string::npos);
				break;
			}
		}
	}
	
	if(!(fEq || fAdd || fMult)){
		if((input[0]=='(')&&(input[inl-1]==')')){
			input.erase(input.begin());
			input.erase(input.end()-1);
			return OKU(input);
		}
	}
	
	stack<int> st1 = OKU(subs1);
	stack<int> st2 = OKU(subs2);
	rslt.push(-1*c);
	while(!st2.empty()){
		int k = st2.top();
		st2.pop();
		temp.push(k);
	}
	while(!temp.empty()){
		int k = temp.top();
		temp.pop();
		rslt.push(k);
	}
	while(!st1.empty()){
		int k = st1.top();
		st1.pop();
		temp.push(k);
	}
	while(!temp.empty()){
		int k = temp.top();
		temp.pop();
		rslt.push(k);
	}
	return rslt;
}

string YAZ(stack<int> &postfix){
	string rslt="", k;
	int fEq = postfix.top();
	if(fEq){ 
		postfix.pop(); k = (char)(-1*postfix.top() );
		rslt += ("\tPUSH V" + k + " ;address of " + k + "\n");
	}
	postfix.pop();
	while(!postfix.empty()){
		int current = postfix.top();
		if(current<0){ //CURRENT was originally a character
			switch(-1*current){
				case 42: k = "\tPOP CX  ;start to multiply\n\tPOP AX\n\tMUL CX\n\tPUSH AX\n"; break;
				case 43: k = "\tPOP CX  ;start to add\n\tPOP AX\n\tADD AX, CX\n\tPUSH AX\n"; break;
				case 45: k = "\tPOP CX  ;start to subtract\n\tPOP AX\n\tSUB AX, CX\n\tPUSH AX\n"; break;
				case 47: k = "\tMOV DX,0;start to divide\n\tPOP CX\n\tPOP AX\n\tDIV CX\n\tPUSH AX\n"; break;
				case 61: /*CODE TO ASSIGN*/
					k = "\tPOP AX  ;start to assign\n";
					k += "\tPOP BX\n\tMOV [BX],AX\n";
					break;
				default: 
					k = "\tPUSH [V"; k += (char)(-1*current); k += "]\n";
					break;
			}
			rslt += k;
		}
		else {  rslt += ("\tPUSH "+to_string(current)+'\n'); }
		postfix.pop();
	}
	if(!fEq){ rslt += "\tPOP AX\n\tCALL MYPRINT\n";	}
	return rslt;
}

string HEADER_YAZ(string &decl){
	string rslt = "JMP START\n\nMYPRINT:\t;COPIED FROM LECTURE NOTES\n\tMOV SI, 10\n\tMOV DX, 0\n\tPUSH 10\n";
	rslt += "\tMOV CX,1\n    NONZERO:\n\tDIV SI\n\tADD DX, 48\n\tPUSH DX\n\tINC CX\n\tMOV DX, 0\n";
	rslt += "\tCMP AX, 0\n\tJNE NONZERO\n    WRITELOOP:\n\tPOP DX\n\tMOV AH, 02H\n\tINT 21H\n";
	rslt += "\tDEC CX\n\tJNZ WRITELOOP\n\tRET\n\n";
	rslt += ("V"+string(1, decl[0]))+":\tDW ?\n";
	for(unsigned int i=1; i<decl.size(); i++){ 
		if(decl[i]!=decl[i-1]){ rslt += ("V"+string(1, decl[i]))+"\tDW ?\n"; } 
	}
	rslt += "\nSTART:\n";
	return rslt;
}

int main(){
	string decision="";
	do{
		string str="", output="", fileloc="example.ac", declarations="";
		cout << "Enter file location/name:\t"; cin >> fileloc;
		if(fileloc[fileloc.length()-1]=='c' 
		&& fileloc[fileloc.length()-2]=='a'
		&& fileloc[fileloc.length()-3]=='.'){
			ifstream ifs;
			ifs.open(fileloc, ifstream::in);
			ofstream ofs;
			fileloc.erase(fileloc.end()-2,fileloc.end()); fileloc += "asm";
			ofs.open(fileloc);
			char c = ifs.get();
			while (ifs.good()) {
				while(c!='\n'){ 
					if(c!=' '){ str += c; }
					c = ifs.get();
				}
				if(str[1]=='='){ declarations += str[0]; }
				stack<int> linepostfix = OKU(str);
				linepostfix.push((str[1]=='='));
				str="";
				output += (YAZ(linepostfix) + ";~~line change in original file~~\n");
				c = ifs.get();
			}
			ifs.close();
			sort(declarations.begin(),declarations.end());
			ofs << HEADER_YAZ(declarations);
			
			ofs << output;
			ofs << "\tINT 20h";
			ofs.close();
			cout << "Output can be found at:\t\t" << fileloc << endl;
		}
		else cout << "Invalid file. Accepted extension: .ac\n";
		cout << "Do you have other files? (Y to continue, N to terminate)" << endl;
		cin.ignore();
		do{
			getline(cin, decision);
			if(decision.size()==1 &&
				(decision[0]=='Y'|| decision[0]=='y'|| decision[0]=='N'|| decision[0]=='n')){ break; }
			else cout << "Enter: Y/N\n";
		} while(true);
	} while (decision[0]=='Y' || decision[0]=='y');
	
	return 0;
}
