
#include "debug.h"
#include "utils.h"
#include "memoryManage.h"

int main(int argc,char** argv)
{
	init_memory();
	init_utils();
	//init_comm();
	
	char str[] = "ab\\r\\ncdef\\r\\rdef\\n\\rdde\\r\\n";
	utils.parse_string_escape_char(str);
	for (int i = 0; str[i]; i++)
	{
		printf("%c", str[i]);
	}
}