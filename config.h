#include <string>
#include <cmath>

using namespace std;

struct parameters{
    string soundFolder;
    string soundList;
    string featParams;
    string featData;
    string classList;
    string model;
    string robotIP;
    int robotPort;
    int windowSamples;
    int subSamples;

    void fill(string *p){
        soundFolder = p[0];
        soundList= p[1];
        featParams= p[2];
        featData= p[3];
        classList = p[4];
        model= p[5];
        robotIP= p[6];
        robotPort= stoi(p[7]);
        windowSamples= pow(2,stoi(p[8]));
        subSamples= pow(2,stoi(p[9]));
    }

};
