#include "shapp.h"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
	std::string command = "Assets\\Shaders\\compile.bat";

	int result = std::system(command.c_str());

	ShAPP app{};

	try 
	{
		app.run();
	}
	catch (const std::exception& e) 
	{
		std::cerr << e.what() << '\n';

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}