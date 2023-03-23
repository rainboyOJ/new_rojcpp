//实现Callback,非虚类动态函数
#include <iostream>


class Callback {
    public:
        using CallbackPtr = void(*)(Callback *,int);

        void callback(int num) {
            if( m_callback)
                this->m_callback(this,num);
        }

    protected:
        CallbackPtr m_callback = nullptr;

};

class CallbackImplBase : public Callback {

public:
    template<typename SubClass>
    CallbackImplBase(SubClass *) {
        m_callback = invoke_subclass_on_callback<SubClass>;
    }

    template<typename SubClass>
    static void invoke_subclass_on_callback(Callback * ptr ,int num) {
        auto * SubClassPtr = static_cast<SubClass *>(ptr);
        SubClassPtr->on_callback(num);
    }
};

class c1 :public CallbackImplBase {
public:

    c1() : CallbackImplBase(this)
    {}
    
    void on_callback(int num) {
        std::cout << "in class c1,num = " << num << "\n";
    }

};

class c2 :public CallbackImplBase {

public:
    c2() : CallbackImplBase(this)
    {}
    
    void on_callback(int num) {
        std::cout << "in class c2,num = " << num << "\n";
    }

};

int main(){
    
    c1 __c1;
    c2 __c2;

    Callback * callback_ptr_arr[] = {&__c1,&__c2};
    callback_ptr_arr[0]->callback(1);
    callback_ptr_arr[1]->callback(2);
    return 0;
}

