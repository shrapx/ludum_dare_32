#include "app.hpp"
#include <string>

int main(int argc, char **argv) {

	std::string path = _WIN64 ?  "data\\config.json" : "data/config.json";

	App a;

	a._config( argc>1 ? argv[1] : path );

  return a._execute();
}
