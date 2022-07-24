#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <ascent.hpp>
#include <conduit.hpp>
#include <algorithm>
#include <conduit_relay.hpp>

using namespace std;

int nodeNum, element;
string title, zone, zoneType;
vector<string> variables;
vector<vector<double> > value;
vector<int> vertexPos;

int s2i(string str){
    int res = 0;
    stringstream ss;
    ss << str;
    ss >> setprecision(16) >> res;
    return res;
}

double s2d(string str){
    double res = 0;
    stringstream ss;
    ss << str;
    ss >> setprecision(32) >> res;
    return res;
}

string dataSplit(string str){
    string res = "";
    int indexBegin = -1;
    int index = str.find("=");
    while(index != -1){
        res = res + string(str.begin() + (indexBegin + 1), str.begin() + index);
        res = res + " " + "=" + " ";
        indexBegin = index;
        index = str.find("=", index + 1);
    }
    res = res + string(str.begin() + (indexBegin + 1), str.begin() + str.size());
    return res;
}

int main(){
    ifstream in;
    in.open("./flowfield.dat", ios::in); // /home/hoody/BUPT-ASCENT/src/examples/DataConvert/flowfield.dat
    if(!in)
        cout << "file is not open!" << endl;
    string line;
    int count = 0;
    while(getline(in, line)){
        stringstream ss;
        vector<string> vs;
        string temp;
        line = dataSplit(line);
        ss << line;
        while(ss >> temp)
        {
            if(temp[temp.size() - 1] == ',')
                temp = temp.substr(0 ,temp.size() - 1);
            vs.push_back(temp);
            // cout << "temp = " << temp << endl;
        }
        if(vs.size()){
            string upper = vs[0];
            transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            if(upper == "TITTLE"){
                title = vs[2];
                cout << title << endl;
            }
            else if(upper == "VARIABLES"){
                variables.push_back(vs[2]);
                for(int i = 1; i < 12; i++){
                    getline(in, line);
                    line = string(line.begin() + 1, line.end() - 2);
                    variables.push_back(line);
                    cout << variables[i] << endl;
                }
            }
            else if(upper == "ZONE"){
                zone = vs[3];
                cout << zone << endl;
                getline(in, line); // STRANDID=0, SOLUTIONTIME=0
            }
            else if(upper == "NODES"){
                nodeNum = s2i(vs[2]);
                 cout << nodeNum << endl;
                element = s2i(vs[5]);
                // cout << element << endl;
                zoneType = vs[8];
                // cout << zoneType << endl;
                getline(in, line); // DATAPACKING=POINT
                getline(in, line); // FACENEIGHBORCONNECTIONS=737908
                getline(in, line); // FACENEIGHBORMODE=LOCALONETOONE
                getline(in, line); // FEFACENEIGHBORSCOMPLETE=YES
                getline(in, line); // DT=(SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE SINGLE )
                count = 0;
            }
            else{
                if(count < nodeNum){
                    value.resize(12);
                    for(int i = 0; i < 12; i++){
                        // cout << s2d(vs[i]) << endl;
                        value[i].push_back(s2d(vs[i]));
                        // if(count == 1){
                        //     cout << s2d(vs[i]) << endl;
                        // }
                    }
                }
                else if(count >= nodeNum && count < nodeNum + element){
                    vertexPos.push_back(s2i(vs[0]));
                    vertexPos.push_back(s2i(vs[1]));
                    vertexPos.push_back(s2i(vs[2]));
                    vertexPos.push_back(s2i(vs[3]));
                }
                count++;
            }
        }
    }
    in.close();


    conduit::Node n;
    n["coordsets/coords/type"] = "explicit";
    n["coordsets/coords/values/x"] = value[0];
    n["coordsets/coords/values/y"] = value[1];
    n["coordsets/coords/values/z"] = value[2];
    n["topologies/mesh/type"] = "unstructured";
    n["topologies/mesh/coordset"] = "coords";
    n["topologies/mesh/elements/shape"] = "tet";
    n["topologies/mesh/elements/connectivity"].set(vertexPos);

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
     conduit::relay::io::load("tet1.yaml", n);
    ascent::Ascent a;
    a.open();
    a.publish(n);

    conduit::Node actions;
    conduit::relay::io::load("test_tet_ascent.yaml", actions);
    a.execute(actions);
    a.close();
    return 0;
}