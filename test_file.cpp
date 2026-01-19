#include <stdio.h>
#include <iostream>
#include <cmath>
using namespace std;
class Solution
{
public:
    char findTheDifference(string s, string t)
    {
        char c = t[s.size()];
        cout << c << endl;
        return t[-1];
    }
};

int main(int agrs, char **kargs)
{
    Solution s;
    s.findTheDifference("abcd", "abcde");
    return 0;
}