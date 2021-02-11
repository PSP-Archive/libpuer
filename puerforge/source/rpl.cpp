#include <vector>
#include <string>
//#include <iostream>
#include <cmath>

#include "../../libpuer/libpuer.h"

using namespace std;

// http://goodjob.boy.jp/chirashinoura/id/100.html
vector<string> split(string &str, char *delim){
	vector<string> result;
	int cutAt;
	while( (cutAt = str.find_first_of(delim)) != str.npos ){
		if(cutAt > 0){
			result.push_back(str.substr(0, cutAt));
		}
		str = str.substr(cutAt + 1);
	}
	if(str.length() > 0){
		result.push_back(str);
	}
	return result;
}

extern "C" void reversepolandTest(){
	int i;

	pspDebugScreenClear();
	pspDebugScreenSetXY(0, 0);
	pspDebugScreenPrintf("RPL test\n\"3 4 + 9 * 7 2 1 + - / 3 **\"\n");
	string RPL="3 4 + 9 * 7 2 1 + - / 3 **";

	vector<string> token = split(RPL," ");
	vector<double> resolveNumber;
	vector<string> resolveString;
	for(i=0;i<token.size();i++){
		char *p=NULL;
		double d=strtod(token[i].c_str(),&p);
		if(p-token[i].c_str()==token[i].size()){
			//pspDebugScreenPrintf("%s number\n",token[i].c_str());
			resolveNumber.push_back(d);
			resolveString.push_back(token[i]);
		}else{
			//pspDebugScreenPrintf("%s op\n",token[i].c_str());
			if(resolveNumber.size()<2){pspDebugScreenPrintf("stack few\n");return;}
			double d2=resolveNumber[resolveNumber.size()-1],d1=resolveNumber[resolveNumber.size()-2];
			resolveNumber.pop_back(),resolveNumber.pop_back();
			string s2=resolveString[resolveString.size()-1],s1=resolveString[resolveString.size()-2];
			resolveString.pop_back(),resolveString.pop_back();
			double ret;

			if(token[i]=="+"){ret=d1+d2;}
			else if(token[i]=="-"){ret=d1-d2;}
			else if(token[i]=="*"){ret=d1*d2;}
			else if(token[i]=="/"){ret=d1/d2;}
			//else if(token[i]=="%"){ret=d1%d2;}
			else if(token[i]=="**"){ret=pow(d1,d2);}
			else{pspDebugScreenPrintf("unknown OP %s\n",token[i].c_str());return;}

			resolveNumber.push_back(ret);
			resolveString.push_back("("+s1+token[i]+s2+")"); //this won't emit "optimized" expression. full of parens.
		}
	}
	if(resolveNumber.size()!=1){pspDebugScreenPrintf("result stack not 1");return;}
	pspDebugScreenPrintf("%s = %.6f\n",resolveString[0].c_str(),resolveNumber[0]);
}
