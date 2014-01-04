

#include <iostream>
#include <vector>
using namespace std;

int main()
{
    vector<int> a(5,0);
    int i;
    for(i=0; i<5; i++){
        a[i] = i;
    }
   cout << " i = " << i << endl;

    cout << a.back() << endl;
}
