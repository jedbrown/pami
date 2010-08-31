#ifndef _PAMI_MATH_H
#define _PAMI_MATH_H

namespace PAMI
{
    namespace Math {
        template <class T> 
            T GCD(T a, T b) 
            {
                T t;
                while (b != 0) {
                    t = b;
                    b = a % b;
                    a = t;
                }
                return a;
            }
    }
}

#endif // _PAMI_MATH_H
