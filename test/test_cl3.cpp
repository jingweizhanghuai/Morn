// g++ test_cl3.cpp -o test_cl3.exe -I ..\..\download\triSYCL-master\include\ -I C:\ProgramFiles\CPackage\boost\include\ -std=c++17

#include <CL/sycl.hpp>
#include <iostream>

using namespace cl::sycl;

constexpr size_t N = 2000;
constexpr size_t M = 3000;

int main() 
{
    queue q;
    buffer<float, 2> a { { N, M } };
    buffer<float, 2> b { { N, M } };
    buffer<float, 2> c { { N, M } };

    q.submit([&](handler &cgh)
    {
        auto A = a.get_access<access::mode::write>(cgh);
        cgh.parallel_for<class init_a>({ N, M },[=](id<2> index){A[index] = index[0]*2 + index[1];});
    });
    
    q.submit([&] (handler &cgh) 
    {
        auto B = b.get_access<access::mode::write>(cgh);
        cgh.parallel_for<class init_b>({ N, M },[=](id<2> index){B[index] = index[0]*2014 + index[1]*42;});
    });

    q.submit([&] (handler &cgh) 
    {
        auto A = a.get_access<access::mode::read>(cgh);
        auto B = b.get_access<access::mode::read>(cgh);
        auto C = c.get_access<access::mode::write>(cgh);
        cgh.parallel_for<class matrix_add>({ N, M },[=](id<2> index){C[index] = A[index] + B[index];});
    });
    
    auto C = c.get_access<access::mode::read>();

    std::cout << std::endl << "Result:" << std::endl;
    for (size_t i = 0; i < N; i++) for(size_t j = 0; j < M; j++)
    {
        if (C[i][j] != i*(2 + 2014) + j*(1 + 42)) 
        {
          std::cout << "Wrong value " << C[i][j] << " on element "<< i << ' ' << j << std::endl;
          return 1;
        }
    }
    std::cout << "Accurate computation!" << std::endl;
    return 0;
}