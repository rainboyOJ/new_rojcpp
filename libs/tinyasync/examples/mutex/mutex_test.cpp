#include <iostream>
#include <bitset>
#include "tinyasync.h"
using namespace std;
using namespace tinyasync;


int main()
{
    cout << " start \n";
    tinyasync::LockCore myLock;
    cout << " before m_flag " << bitset<5>(myLock.m_flags.load()) << "\n";

    
    ListNode ln_a;
    bool ret = myLock.try_lock(&ln_a);

    cout << " try_lock : " << ret << "\n ";
    cout << "after m_flag " << bitset<4>(myLock.m_flags.load()) << "\n";

    ret = myLock.try_lock(&ln_a);
    cout << " try_lock : " << ret << "\n ";
    cout << "after m_flag " << bitset<4>(myLock.m_flags.load()) << "\n";

    ret = myLock.try_lock(&ln_a);
    cout << " try_lock : " << ret << "\n ";
    cout << "after m_flag " << bitset<4>(myLock.m_flags.load()) << "\n";

    return 0;
}