#include "interface.h"

vector<FFISafeExpr*> TranslateRustVec(FFISafeExprVec rustVec) {
    vector<FFISafeExpr*> vec;
    for (int i = 0; i < rustVec.len; i++) {
        vec.push_back(rustVec.ptr + i);
    }
    return vec;
}

int main() {
    FFISafeExprVec tokens = recieve_tokens();
    vector<FFISafeExpr*> vec = TranslateRustVec(tokens);
    for (auto item : vec) {
        cout << item->function_definition._1;
    }    
}