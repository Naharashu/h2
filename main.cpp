#include <iostream>
#include "h2.hpp"

int main() {
	std::vector<{"h", "e", "l", "l", "o", "!", "\0"};
	uint256 res = h2_hash(msg);
	std::cout << res.a << res.b << res.c << res.d << '\n';
	return 0;
}
