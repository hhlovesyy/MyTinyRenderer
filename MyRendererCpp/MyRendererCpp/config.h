#ifndef CONFIG_H
#define CONFIG_H

class Global_Config
{
public:
	static bool should_flip_uv_y;
	static void SetFlipUVY(bool isFlip)
	{
		should_flip_uv_y = isFlip;
	}
};

#endif