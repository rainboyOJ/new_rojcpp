/**
 * 测试 toos/buffers.h的使用
 */

#include <iostream>
#include "tools/buffers.h"

//定义一个自己的 memory_resource
//本质是调用 new_delete_resource
//目的,了解调用各个函数的时间与过程
class memory_resource_hook : public std::pmr::memory_resource {
    
public:

    static std::pmr::memory_resource * ndr ; //= std::pmr::new_delete_resource();
    
    memory_resource_hook() : memory_resource() {}

    
private:

    virtual void * do_allocate(std::size_t bytes, std::size_t alignment = 0)  override
    {
        std::cout << "do_allocate" << "\n";
        return ndr->allocate(bytes, alignment);
    }

    virtual void do_deallocate(void *p ,size_t bytes, size_t alignment) override
    {
        std::cout << "do_deallocate " << "\n";
        return ndr->deallocate(p, bytes, alignment);
    }
    
    virtual bool do_is_equal (const std::pmr::memory_resource & other) const noexcept override
    {
        return true;
    }
};

std::pmr::memory_resource * memory_resource_hook::ndr 
                = std::pmr::new_delete_resource();


int main(){

    //创建一个 memory_resource_hook 的实例
    memory_resource_hook myndr;

    //创建Buffer
    rojcpp::Buffer myBuffer(&myndr);

    // ==== 测试 myBuffer 的使用
    
    auto cout_myBuffer_info  = [&myBuffer](std::string_view info = "") {
        std::cout << "info >>> "<< info << "\n";
        std::cout << "myBuffer is empty ? -> " << myBuffer.empty()  << "\n";
        std::cout << "myBuffer tot size? -> " << myBuffer.tot_size()  << "\n";
        std::cout << "myBuffer used size? -> " << myBuffer.used_size()  << "\n";
        std::cout << "myBuffer left_size? -> " << myBuffer.left_size()  << "\n";
    };

    cout_myBuffer_info("after create myBuffer");

    myBuffer.expand_size(8);
    cout_myBuffer_info("after myBuffer.expand_size(8)");

    char chArr[] = "12345678";
    memcpy(myBuffer.write_data(), chArr, myBuffer.left_size());
    myBuffer.update_used_sized(8); //更新写入的数据大小

    cout_myBuffer_info("after write myBuffer data: \"12345678\" ");

    typedef char ai[8]; // ai的类型是  char [8]
                        // <=> ai是有8个元素的char数组类型
                        // ai * 是就是一个指向 char [8]的指针

                        // 数组指针

    ai * a = (ai*)myBuffer.data();
    for(int i = 0 ;i < 8;i++) {
        std::cout << (*a)[i] << " "; //输出数组里的元素
    }


    return 0;
}
