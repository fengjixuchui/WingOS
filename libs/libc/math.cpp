#include <math.h>
#include <stdint.h>

#ifdef __SSE__
double pow(double x, double y)
{

    if (y == 0)
    {
        return 1;
    }
    else if (isnan(x) || isnan(y))
    {
        return x;
    }
    else if (x == 0)
    {
        return 0;
    }
    else if (y == 1)
    {
        return x;
    }
    int res = (int)y;
    if (res == y)
    {

        double dresult = x;
        double yabs = fabs(y);
        for (size_t count = 0; count < yabs; count++)
        {
            dresult *= x;
        }
        if (yabs < 0)
        {
            return (double)1 / dresult;
        }
        return res;
    }
    else
    {
        // not supported for the moment
        return 0;
    }
}

float powf(float x, float y)
{
    if (y == 0)
    {
        return 1;
    }
    else if (isnan(x) || isnan(y))
    {
        return x;
    }
    else if (x == 0)
    {
        return 0;
    }
    else if (y == 1)
    {
        return x;
    }
    int res = (int)y;
    if (res == y)
    {

        float dresult = x;
        float yabs = fabs(y);
        for (size_t count = 0; count < yabs; count++)
        {
            dresult *= x;
        }
        if (yabs < 0)
        {
            return (float)1 / dresult;
        }
        return res;
    }
    else
    {
        // not supported for the moment
        return 0;
    }
}
#ifdef X87
long double powl(long double x, long double y)
{
    if (y == 0)
    {
        return 1;
    }
    else if (y == 2)
    {
        return x * x;
    }
    else if (y == 1)
    {
        return x;
    }
    long double res = 1;
    for (unsigned int count = 0; count < y; count++)
    {
        res *= x;
    }
    return res;
}
#endif

double trunc(double x)
{
    if (x > 0)
    {
        return floor(x);
    }
    return ceil(x);
}

double floor(double x)
{
    if (isnan(x) || isinf(x))
    {
        return x;
    }
    long long ret = (long long)x;
    double ddret = (double)ret;
    if (ddret == x || x >= 0)
    {
        return ddret;
    }
    return ddret - 1;
}

double ceil(double x)
{
    if (isnan(x) || isinf(x))
    {
        return x;
    }
    long long ret = (long long)x;
    double ddret = (double)ret;
    if (ddret == x || x >= 0)
    {
        return ddret;
    }
    return ddret + 1;
}

double fabs(double x)
{
    if (x < 0)
    {
        return -x;
    }
    return x;
}

double sin(double x)
{
    int ix = 1;
    double current = x;
    double accurency = 1;
    double factor = 1;
    double power = x;
    while (fabs(accurency) > 0.00001 && ix < 100)
    {
        factor *= ((2 * ix) * (2 * ix + 1));
        power *= -1 * ix * ix;
        accurency = power / factor;
        current += accurency;
        ix++;
    }
    return current;
}

double cos(double x)
{
    return sin(x + PI / 2);
}
double ldexp(double x, int exp)
{
    if (exp == 0)
    {
        return x;
    }
    else if (isnan(x))
    {
        return x;
    }
    else if (isinf(x))
    {
        return x;
    }
    else
    {
        return x * (2 ^ exp);
    }
}
float ldexpf(float x, int exp)
{
    if (exp == 0)
    {
        return x;
    }
    else if (isnan(x))
    {
        return x;
    }
    else if (isinf(x))
    {
        return x;
    }
    else
    {
        return x * (2 ^ exp);
    }
}
#ifdef X87
long double ldexpl(long double x, int exp)
{

    if (exp == 0)
    {
        return x;
    }
    else if (isnan(x))
    {
        return x;
    }
    else if (isinf(x))
    {
        return x;
    }
    else
    {
        return x * (2 ^ exp);
    }
}
#endif

#endif
