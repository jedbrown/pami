/**
 * \file common/type/Math.h
 * \brief ???
 */
#ifndef __common_type_Math_h__
#define __common_type_Math_h__

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
