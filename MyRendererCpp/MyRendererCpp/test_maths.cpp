#include "test_maths.h"
#include "maths.h"
#include <iostream>

#include <iostream>
void test_all_maths()
{
	//主要用于测试新的数学库接口
	vec3 a{ 1, 2, 3 };
	vec3 b{ 4, 5, 6 };
	float res = dot(a, b);
	vec3 c = cross(a, b);
	std::cout << res << std::endl;

}