#include <iostream>

#include "Application.h"
#include "framework/Renderer.h"

int main()
{
	Renderer::get();
	Application app;
	app.run();
}

//improve render pass code
//add shadows