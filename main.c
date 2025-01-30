#include <stdio.h>

#include "double_endianess_decoder.h"


void print_internal_repr(double input){
	unsigned char * bytes = (void*)&input;
	for(int i=0; i<8; i++){
		printf("[%03u]", (unsigned)bytes[i]);
	}
	putchar('\n');

}

int main(void){


	double litmus_val = 8.207880399131839300000000000000E-304;

	double test_val = litmus_val;

	puts("This is double endianess decoder:");
	printf("Test value:\n%.20E\n", test_val);
	print_internal_repr(test_val);

	printf("\nAs long as platform supports IEEE-754 floating point representation,\n"
		"It lets you convert a double from whatever endianess platform supports to either:\n");

	printf("- Little Endian\n%.20E\n", dbl_system_to_LE(test_val));
	print_internal_repr(dbl_system_to_LE(test_val));
	printf("- Big Endian\n%.20E\n", dbl_system_to_BE(test_val));
	print_internal_repr(dbl_system_to_BE(test_val));
	printf("- Bytes Reversed\n%.20E\n", dbl_bytes_reverse(test_val));
	print_internal_repr(dbl_bytes_reverse(test_val));

	printf("\nLibrary also supports inverse conversions:\n");
	printf("- From Little Endian to system's \n%.20E\n", dbl_LE_to_system(dbl_system_to_LE(test_val)));
	print_internal_repr(dbl_LE_to_system(dbl_system_to_LE(test_val)));
	printf("- From Little Endian to system's \n%.20E\n", dbl_BE_to_system(dbl_system_to_BE(test_val)));
	print_internal_repr(dbl_BE_to_system(dbl_system_to_BE(test_val)));



	printf("\nThe libarary header is made to work for a wide range of platforms and if possible, generate efficient code.\n"
		"Little and Big endian are not the only endianesses supported, the library functions will accept any endianess possible,\n"
		"including esoteric made-up ones such as AHCDFEBG. This is thanks to the hard coded \"litmus\" value you saw in the test.\n"
		"Each byte of litmus encodes Big endian indexes starting from 1, so functions can determine how to convert back and forth using it as a reference.\n"
		"See readme and 'double_litmus_value.png' for more info.");



}