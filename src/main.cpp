#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <iostream>

int main(int argc, char* argv[]) {
	spdlog::set_level(spdlog::level::debug);
	spdlog::debug("Hello, {}!", "World");
	spdlog::info("Hello, World!");
	// print 当前level

	return 0;
}