#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "protocols.h"
#include "mysocket.h"
using namespace std;

struct Point{
    int x;
    int y;
};

int main(){
    Endpoint ep(TCP | SERVER, "192.168.1.114", "5000");

    int n;
    while(true){
        string buf("");
        ep.Read(-10, buf);
        if(buf == "") break;
        else if(buf == "pass"){
            cout << "level " << n << " pass" << endl;
            continue;
        }
        stringstream ss(buf);
        ss >> n;

        string s(to_string(n));
	    while (s.length() < 3) s = "0" + s;
	    string map_name = "map" + s + ".txt";
        //cout << map_name << endl;

        ifstream fin(map_name);
	    if (fin.fail()) {
		    ep.Write("error");
            continue;
	    }

        string line;        
	    while (getline(fin, line)) {
            //cout << line << endl;
            Sleep(10);
            ep.Write(line);
	    }
        ep.Write("end");
    }

    return 0;
}