#include "stdshit.h"
#include "peFile/peFile.h"
#include "rossym.h"
const char progName[] = "rossym-conv";

PeFile peFile;

cch* prevName;

void add_symbol(cch* name, cch* fileName, int addr)
{
  // exclude duplicates
  if(prevName && !strcmp(prevName, name)) return;
  prevName = name;

  // add valid symbol
  if(peFile.rvaToSect(addr, 0))
    peFile.symtab.add((char*)name, addr);
}

BOOL parse_rossym(const void* rsym_ptr, int rsymlen)
{
    const ROSSYM_HEADER* RosSymHeader;
    const ROSSYM_ENTRY* First, *Last, *Entry;
    const CHAR* Strings;

    RosSymHeader = (ROSSYM_HEADER*)rsym_ptr;

    if (RosSymHeader->SymbolsOffset < sizeof(ROSSYM_HEADER)
        || RosSymHeader->StringsOffset < RosSymHeader->SymbolsOffset + RosSymHeader->SymbolsLength
        || rsymlen < RosSymHeader->StringsOffset + RosSymHeader->StringsLength
        || 0 != (RosSymHeader->SymbolsLength % sizeof(ROSSYM_ENTRY)))
    {
        printf("Invalid ROSSYM_HEADER\n");
        return FALSE;
    }

    First = (const ROSSYM_ENTRY *)((const char*)rsym_ptr + RosSymHeader->SymbolsOffset);
    Last = First + RosSymHeader->SymbolsLength / sizeof(ROSSYM_ENTRY);
    Strings = (const CHAR*)rsym_ptr + RosSymHeader->StringsOffset;

    for (Entry = First; Entry != Last; Entry++)
    {
      const char* SymbolName = Strings + Entry->FunctionOffset;
      const char* FileName = Strings + Entry->FileOffset;
      add_symbol(SymbolName, FileName, Entry->Address);
    }

    return TRUE;
}

int main(int argc, char* argv[])
{
	// get arugments
    if(argv[1] == NULL) {
        printf("rossym-conv <dest file> [source file]\n");
        return -1; }
    cch* outFile = argv[1]; cch* inFile = argv[2];
    if(inFile == NULL) inFile = outFile;

	// load input pe file
	cch* err = peFile.load(inFile);
	if(err) { printf("failed to load input file: %s\n", err);
		return 1; }

	// locate .rossym
	for(auto& sect : peFile.sects) {
		if(!strcmp(sect.name, ".rossym")) {
			if(!parse_rossym(sect.data, sect.len))
                return 3;
			goto FOUND_ROSSYM;
        }
	}

	// locate .rossym failed
	printf(".rossym section not found\n");
	return 3;

FOUND_ROSSYM:
	// save output pe file
	if(peFile.save(outFile)) {
		printf("failed to save output file\n");
		return 2; }
	return 0;
}
