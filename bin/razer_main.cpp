#include "razer.h"
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

void render_main(shared_ptr<Razer>& razer, const string& out){
    string lrc;
    char buff[256];
    while(cin >> lrc){
        long l, r, c;
        sscanf(lrc.c_str(), "%ld,%ld,%ld", &l, &r, &c);
        try{
            auto rslt = razer->render_png(l, r, c);
            if(rslt.empty()) printf("failed render %s\n", lrc.c_str());
            else {
                sprintf(buff, "%s_%ld_%ld_%ld.png", out.c_str(), l, r, c);
                auto fp = fopen(buff, "w+");
                fwrite(rslt.c_str(), 1, rslt.length(), fp);
                fclose(fp);
            }
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

    auto razer = Razer::GetRazer(style);
    render_main(razer, output);
    return 0;
}
