#include "Renderer.h"
#include "HUD.h"
#include <vector>


HUD::HUD(Renderer& r, int w, int h) : renderer(r), screenW(w), screenH(h)
{
	
}

void HUD::drawCrosshair()
{
	int leng = 10; //length of the crosshair lines(half actual value)
	for (int i = -leng;i <= leng;i++)
	{
		for (int j = -1;j <= 1;j++)
		{
			renderer.setPixel((screenW / 2)+i, (screenH / 2)+j, 155, 155, 155);
			renderer.setPixel((screenW / 2) + j, (screenH / 2) + i, 155, 155, 155);
		}
	}
	
}
void HUD::drawBars(int health, int maxHealth, float stamina, float maxStamina) 
{
	float HP = glm::clamp(((float)health / (float)maxHealth), 0.0f, 1.0f);
	float ST = glm::clamp((stamina / maxStamina), 0.0f, 1.0f);
	
	//Bar BG
	renderer.drawRect(10, 10, 250, 25, 100, 100, 100);
	renderer.drawRect(10, 40, 250, 25, 100, 100, 100);

	//CurrentHP
	renderer.drawRect(14, 13, 242*HP, 19, 255, 0, 0);
	//stamina
	renderer.drawRect(14, 42, 242 * ST, 19, 0, 255, 0);
}