#include <stdio.h>
#include <stdlib.h>
#include <libiberty/demangle.h>

char* demangle(const char* name)
{
	// parse mangled name
	void* mem = 0;
	struct demangle_component* tree;
	tree = cplus_demangle_v3_components(
		name, 0, &mem);

	// print demangled name
	size_t size = 0;
	char* str = cplus_demangle_print(0, tree, size, &size);
	free(mem);
	return str;
}
