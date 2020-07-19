#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Greska: Potrebno je proslediti bar 1 ulazni fajl" << endl;
        return 0;
    }

    cout << "Prosledili ste fajlove: ";
    for (int i = 1; i < argc; i++) {
        if (i != 1) cout << ", ";
        cout << argv[i];
    }

    cout << endl;

    return 0;
}
