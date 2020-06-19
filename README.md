#Preface#
This is a memory allocator that is written in C++, it does not use malloc, it uses the sbrk() system call to expand and shorten the heap as needed. This was just a little side project I did to sharpen my programming skills, and while
this memory allocator works, you're probably better off using malloc or a custom built allocator.

##Building##
In order to use this allocator with your program, all you have to do is clone the repository, include __Allocator.h__ found in the __include__ directory, and compile with __Allocator.cpp__ found in the __src__ directory.

##Using##
To use the allocator made here it is first required that one instantiates the Allocator class. The constructor takes 2 parameters, bit alignment, and initial allocation size. The constructor has defaults, them being, 16 for bit alignment
and 10240 for initial allocation size. To allocate memory simply call the __Allocator::allocate__ method passing the number of bytes wanted to allocate. To deallocate memory use the __Allocator::deallocate__ method, passing the pointer 
given by the original allocation. The Allocator class can only be declared once in the program, if it's declared twice the program will crash.
