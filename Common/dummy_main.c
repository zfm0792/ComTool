
#include "debug.h"
#include "utils.h"
#include "memoryManage.h"

int main(int argc,char** argv)
{
	init_memory();
	init_utils();
	//init_comm();
	
	char str[] = "ab\r\ncdef\r\n\rdef\n\rdde\r\n";
	utils.remove_string_return(str);
	for (int i = 0; str[i]; i++)
	{
		printf("%c", str[i]);
	}
}