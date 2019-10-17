#include "VulkanTriangle.h"

int main() 
{
	VulkanTriangle *app = new VulkanTriangle();
	app->Start();

	delete app;

	return 0;
}