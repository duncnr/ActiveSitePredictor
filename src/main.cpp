#include "app.h"
#include <iostream>

// haskell include
#include <inttypes.h>
extern "C" {
	// TODO: Ideally we would have the build system generate these
	extern void hs_init(int* argc, char** argv[]);
	extern void hs_exit(void);
	extern int32_t haskell_add(int32_t a1, int32_t a2);
}

int main(int argc, char* argv[]) {
	hs_init(&argc, &argv);

	App app = App();

	hs_exit();
	return 0;
}
