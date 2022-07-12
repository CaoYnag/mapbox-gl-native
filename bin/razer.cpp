#include "rasterizer.h"
#include <string>
#include <fstream>
#include <iostream>
using namespace std;


void render_main(Rasterizer& razer, const string& out){
    string lrc;
    while(cin >> lrc){
        long l, r, c;
        sscanf(lrc.c_str(), "%ld,%ld,%ld", &l, &r, &c);
        try{
            auto rslt = razer.render_tile(l, r, c);
            char buff[256];
            sprintf(buff, "%s_%02ld_%ld_%ld.png", out.c_str(), l, r, c);
            ofstream of(buff, ios::binary);
            of << mbgl::encodePNG(rslt);
            of.close();
        } catch(exception& e){
            cerr << "Error: " << e.what() << endl;
        }
    }
}

int main(int argc, char** argv){
    if(argc < 3){
        fprintf(stderr, "Usage: razer style output\n");
        fprintf(stderr, "Exp  : razer smp.style.json tile\n");
        return 1;
    }
    string style = argv[1];
    string output = argv[2];

    Rasterizer razer(style);
    render_main(razer, output);
    return 0;
}
