#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <ascent.hpp>
#include <conduit.hpp>
#include <algorithm>
#include <conduit_relay.hpp>

using namespace std;

int nodeNum, element;
string title, zoneType;
vector<string> variables;
vector<vector<double>> value;
vector<int> vertexPos;

int s2i(string str){
    int res = 0;
    stringstream ss;
    ss << str;
    ss >> res;
    ss.clear;
    return res;
}

double s2d(string str){
    double res = 0;
    stringstream ss;
    ss << str;
    ss >> res;
    ss.clear;
    return res;
}

void ParseData(){
    ifstream in;
    in.open("./flowfield.dat");
    if(!in.is_open())
        cout << "file is not open!" << endl;
    string line;
    int index = 0;
    while(in >> line){
        stringstream ss;
        vector<string> vs;
        string t;
        line >> ss;
        while(ss >> t){
            if(t[t.size() - 1] == ',')
                t = t.substr(0 ,t.size() - 1);
            vs.push_back(t);
        }
        if(vs.size()){
            string upper = vs[0];
            transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            if(upper == "TITTLE"){
                title = vs[2];
            }
            else if(upper == "VARIABLES"){
                variables.push_back(vs[2]);
                for(int i = 1; i < 12; i++){
                    getline(in, line);
                    line = string(line.begin() + 1, line.end() - 2);
                    variables.push_back(line);
                }
            }
            else if(upper == "ZONE"){
                zone = vs[3];
                getline(in, line); // STRANDID=0, SOLUTIONTIME=0
            }
            else if(upper == "NODES"){
                nodeNum = s2i(vs[2]);
                element = s2i(vs[5]);
                zoneType = vs[8];
                getline(in, line); // DATAPACKING=POINT
                getline(in, line); // FACENEIGHBORCONNECTIONS=737908
                getline(in, line); // FACENEIGHBORMODE=LOCALONETOONE
                getline(in, line); // FEFACENEIGHBORSCOMPLETE=YES
                getline(in, line); // DT=(SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE )
            }
            else{
                if(index < nodeNum){
                    for(int i = 0; i < 12; i++){
                        value[i].push_back(s2d(vs[i]));
                    }
                }
                else if(index >= nodeNum && index < nodeNum + element){
                    vertexPos.push_back(s2i(vs[0]));
                    vertexPos.push_back(s2i(vs[1]));
                    vertexPos.push_back(s2i(vs[2]));
                    vertexPos.push_back(s2i(vs[3]));
                }
                index++;
            }
        }
    }
    in.close();
}

int main(){
    ParseData();
    conduit::Node n;
    n["coordsets/coords/type"] = "explicit";
    n["coordsets/coords/values/x"] = Value[0];
    n["coordsets/coords/values/x"] = Value[1];
    n["coordsets/coords/values/x"] = Value[2];
    n["coordsets/mesh/type"] = "unstructured";
    n["coordsets/mesh/coordset"] = "coords";
    n["coordsets/mesh/elements/shape"] = "tet";
    n["coordsets/mesh/elements/connectivity"].set(vertexPos);

    for(int i = 3; i < 12; i++){
        stringstream ss;
        string res;
        ss << "fields/" << variables[i] << "/association";
        ss >> res;
        ss.clear();
        n[res] = "vertex";
        ss << "fields/" << variables[i] << "/topology";
        ss >> res;
        ss.clear();
        n[res] = "mesh";
        ss << "fields/" << variables[i] << "/volume_dependent";
        ss >> res;
        ss.clear();
        n[res] = "false";
        ss << "fields/" << variables[i] << "/values";
        ss >> res;
        n[res].set(value[i]);
    }

    conduit::relay::io::save(n, "tet1.yaml");
    return 0;
}