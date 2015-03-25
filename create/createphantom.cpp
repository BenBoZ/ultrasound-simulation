#include <cstdlib>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "./phantom.h"

using std::cout;
using std::endl;


int main(int argc, char* argv[]) {
    myVector  inSize(0.0, 0.0, 0.0);
    double density, c0, a0, a1, a2;
    int success = 0;
    char strPhanFile[60], strBscFile[60];

    // TODO(Unknown): at the moment an empty feature,
    // need to include backscatter coefficient calculations
    // int scatterType;

    FILE *fpinput;

    if (argc < 2) {
        cout << "Error! An input file is needed" << endl;
        exit(-1);
    }

    if ((fpinput=fopen(argv[1], "r")) == NULL) {
        cout << "Can't find input file!" << endl;
        exit(-1);
    }

    while (fgetc(fpinput) != ':') {}
    success = fscanf(fpinput, "%lf, %lf, %lf", &inSize.x, &inSize.y, &inSize.z);
    assert(success == 3);

    while (fgetc(fpinput) != ':') {}
    success = fscanf(fpinput, "%lf", &density);
    assert(success == 1);

    while (fgetc(fpinput) != ':') {}
    success = fscanf(fpinput, "%lf", &c0);
    assert(success == 1);

    while (fgetc(fpinput) != ':') {}
    success = fscanf(fpinput, "%lf, %lf, %lf", &a0, &a1, &a2);
    assert(success == 3);

    while (fgetc(fpinput) != ':') {}
    success = fscanf(fpinput, "%s", strBscFile);
    assert(success == 1);

    while (fgetc(fpinput) != ':') {}
    success = fscanf(fpinput, "%s", strPhanFile);
    assert(success == 1);

    cout << "The x size is " << inSize.x << endl;
    cout << "The y size is " << inSize.y << endl;
    cout << "The z size is " << inSize.z <<endl;
    cout << "The density is " << density << endl;
    cout << "The filename is " << strPhanFile << endl;
    cout << "The sound Speed is " << c0 << endl;
    cout << "The attenuation is " << a0 <<" " << a1 << "  " << a2 << endl;

    phantom target;
    target.createUniformPhantom(inSize, density, c0, a0, a1, a2, strBscFile);
        target.savePhantom(strPhanFile);
}
