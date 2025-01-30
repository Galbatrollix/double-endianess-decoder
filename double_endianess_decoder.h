#ifndef GALBA_DOUBLE_ENDIANESS_DECODER
#define GALBA_DOUBLE_ENDIANESS_DECODER


// converts from big endian to little endian representation and vice versa
static double dbl_bytes_reverse(double);

// convert double representtion from whatever is the system's into big endian
static double dbl_system_to_BE(double);
// convert double in big endian representation to whatever is the system's repr
static double dbl_BE_to_system(double);

// convert double representtion from whatever is the system's into little endian
static double dbl_system_to_LE(double);
// convert double in little endian representation to whatever is the system's repr
static double dbl_LE_to_system(double);



#include <stdint.h>
#include <limits.h>
// if stdint is not available for some reason, remove the include, theres an implementation that doesnt rely on ints
// That implementation will be chosen if UINT64_MAX is not defined.


#if (CHAR_BIT != 8)
	#error "Machines with byte bit widths different than 8 not supported."
#endif


// https://stackoverflow.com/a/753018
#ifndef _MSC_VER
// MSVC does not define the following macros but should guarantee IEEE representation
#if !((__DBL_MANT_DIG__ == 53) && (__DBL_DIG__ == 15) && (__DBL_MAX_10_EXP__ == 308) && \
	(__DBL_MAX_EXP__ == 1024) && (__DBL_MIN_10_EXP__ == -307) && (__DBL_MIN_EXP__ == -1021))

	#error "Machines not using IEEE 754 format for double not supported."
#endif

#endif



#ifdef UINT64_MAX
    #define GALBA_DED_U64_SUPPORTED
#endif


#if defined __clang__ || defined __GNUC__
	#if defined __has_builtin
        #if __has_builtin(__builtin_bswap64)
            #define GALBA_DED_GGC_BSWAP64 
        #endif
    #endif
#endif  // __clang__ or __GNUC__


#ifdef __GNUC__
    #if defined __has_include
        #if __has_include(<endian.h>)
           	#include <endian.h>   
        #endif  
    #endif 



    #ifdef __FLOAT_WORD_ORDER__
        #if  __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
            #define GALBA_DED_GGC_LITTLE
        #elif  __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
            #define GALBA_DED_GGC_BIG
        #endif
    #endif 

#endif  // __GNUC__



#define DBL_LITMUS 8.207880399131839300000000000000E-304

// takes uchar pointer to DBL_LITMUS value
#define LITMUS_AS_MAPPING(LITMUS) {LITMUS[0] - 1, LITMUS[1] - 1, LITMUS[2] - 1, LITMUS[3] - 1, LITMUS[4] - 1,  \
	 LITMUS[5] -1, LITMUS[6] - 1, LITMUS[7]- 1}  	



// If uint64 is not supported, the only reasonable way to convert is to use the char array directly
// Clang only compiler consistently optimizing such obscure operations is clang
// On most compilers functions defined in this block may be pretty inefficient but they will be correct
#ifndef GALBA_DED_U64_SUPPORTED


double dbl_bytes_reverse(double input){
	unsigned char* input_as_bytes = (unsigned char *)&input;
	for(int i=0; i<4; i++){
		unsigned char temp = input_as_bytes[i];
		input_as_bytes[i] = input_as_bytes[7-i];
		input_as_bytes[7-i] = temp;
	}
	return input;
}


double dbl_system_to_BE(double input){
	const double litmus = DBL_LITMUS;
	const unsigned char order[8] = LITMUS_AS_MAPPING(((const unsigned char*)&litmus));

	double result;
	unsigned char* input_as_bytes = (unsigned char *)&input;
	unsigned char* bytes = (unsigned char *)&result;
	for(int i=0; i<8; i++){
		(bytes)[i] = input_as_bytes[order[i]];
	}
	return result;

}


double dbl_system_to_LE(double input){
	const double litmus = DBL_LITMUS;
	const unsigned char order[8] = LITMUS_AS_MAPPING(((const unsigned char*)&litmus));

	double result;
	unsigned char* input_as_bytes = (unsigned char *)&input;
	unsigned char* bytes = (unsigned char *)&result;
	for(int i=0; i<8; i++){
		(bytes)[i] = input_as_bytes[7 - order[i]];
	}
	return result;

}

double dbl_BE_to_system(double input){
	const double litmus = DBL_LITMUS;
	const unsigned char order[8] = LITMUS_AS_MAPPING(((const unsigned char*)&litmus));
	unsigned char inverse[8];
	for(int i=0; i<8; i++){
		inverse[order[i]] = i;
	}


	double result;
	unsigned char* input_as_bytes = (unsigned char *)&input;
	unsigned char* bytes = (unsigned char *)&result;
	for(int i=0; i<8; i++){
		(bytes)[i] = input_as_bytes[inverse[i]];
	}
	return result;

}

double dbl_LE_to_system(double input){
	const double litmus = DBL_LITMUS;
	const unsigned char order[8] = LITMUS_AS_MAPPING(((const unsigned char*)&litmus));
	unsigned char inverse[8];
	for(int i=0; i<8; i++){
		inverse[order[i]] = i;
	}
	
	double result;
	unsigned char* input_as_bytes = (unsigned char *)&input;
	unsigned char* bytes = (unsigned char *)&result;
	for(int i=0; i<8; i++){
		(bytes)[i] = input_as_bytes[7 - inverse[i]];
	}
	return result;
}

#else /// std uint64 available

	// choosing either intrinsic (if available) or handrolled way to reverse 64 bit unsigned int
	#ifdef GALBA_DED_GGC_BSWAP64
		#define UINT64_BYTES_REVERSE(VAL) do{																		\
			VAL = __builtin_bswap64(VAL);																			\
		} while(0)
	#else
		#define UINT64_BYTES_REVERSE(VAL) do{                                                  					    \
			VAL = ( VAL & (uint64_t)0xFFFFFFFF00000000 ) >> 32 | ( VAL & (uint64_t)0x00000000FFFFFFFF ) << 32;   	\
			VAL = ( VAL & (uint64_t)0xFFFF0000FFFF0000 ) >> 16 | ( VAL & (uint64_t)0x0000FFFF0000FFFF ) << 16;		\
			VAL = ( VAL & (uint64_t)0xFF00FF00FF00FF00 ) >> 8  | ( VAL & (uint64_t)0x00FF00FF00FF00FF ) << 8 ;      \
		}while(0)
	#endif 

	double dbl_bytes_reverse(double input){
		unsigned char* input_as_bytes = (unsigned char *)&input;
		uint64_t as_int;
		unsigned char* int_as_bytes = (unsigned char*)&as_int;
		// shouldnt be necessary to replace with memcpy to enable optimizations
		for(int i = 0; i<8; i++){
			int_as_bytes[i] = input_as_bytes[i];
		}

		UINT64_BYTES_REVERSE(as_int);
		for(int i = 0; i<8; i++){
			input_as_bytes[i] = int_as_bytes[i];
		}

		return input;
	}	


	#ifdef GALBA_DED_GGC_LITTLE  
		double dbl_system_to_BE(double input){ return dbl_bytes_reverse(input);}
		double dbl_BE_to_system(double input){ return dbl_bytes_reverse(input);}

		double dbl_system_to_LE(double input){ return input;}
		double dbl_LE_to_system(double input){ return input;}
	#elif defined(GALBA_DED_GGC_BIG)
		double dbl_system_to_BE(double input){ return input;}
		double dbl_BE_to_system(double input){ return input;}

		double dbl_system_to_LE(double input){ return dbl_bytes_reverse(input);}
		double dbl_LE_to_system(double input){ return dbl_bytes_reverse(input);}

	#else // endianess not known via predefined macros

			// litmus uint is an uint that uses litmus double to check whether double endianess is BE, LE or other
			#define LITMUS_UINT64(LITMUS_BYTES)  (((uint64_t)LITMUS_BYTES[0] << 7*8 ) | ((uint64_t)LITMUS_BYTES[1] << 6*8 ) | \
			((uint64_t)LITMUS_BYTES[2] << 5*8 ) | ((uint64_t)LITMUS_BYTES[3] << 4*8 ) | ((uint64_t)LITMUS_BYTES[4] << 3*8 ) | \
			((uint64_t)LITMUS_BYTES[5] << 2*8 ) | ((uint64_t)LITMUS_BYTES[6] << 1*8 ) | ((uint64_t)LITMUS_BYTES[7] << 0*8 ))

			#define LITMUS_UINT_BE (uint64_t)0x0001020304050607
			#define LITMUS_UINT_LE (uint64_t)0x0706050403020100


			double dbl_system_to_BE(double input){
				const double litmus = DBL_LITMUS;
				const unsigned char order[8] = LITMUS_AS_MAPPING(((const unsigned char*)&litmus));
				unsigned char* input_as_bytes = (unsigned char *)&input;

				if(LITMUS_UINT64(input_as_bytes) == LITMUS_UINT_BE){
					return input;
				}else if(LITMUS_UINT64(input_as_bytes) == LITMUS_UINT_LE){
					return dbl_bytes_reverse(input);
				}

				// base case path when endianess is neither BE or LE
				double result;
				unsigned char* bytes = (unsigned char *)&result;
				for(int i=0; i<8; i++){
					(bytes)[i] = input_as_bytes[order[i]];
				}
				return result;

			}

			double dbl_system_to_LE(double input){
				const double litmus = DBL_LITMUS;
				const unsigned char order[8] = LITMUS_AS_MAPPING(((const unsigned char*)&litmus));
				unsigned char* input_as_bytes = (unsigned char *)&input;

				if(LITMUS_UINT64(input_as_bytes) == LITMUS_UINT_BE){
					return dbl_bytes_reverse(input);
				}else if(LITMUS_UINT64(input_as_bytes) == LITMUS_UINT_LE){
					return input;
				}

				// base case path when endianess is neither BE or LE
				double result;
				unsigned char* bytes = (unsigned char *)&result;
				for(int i=0; i<8; i++){
					(bytes)[i] = input_as_bytes[7 - order[i]];
				}
				return result;

			}


			double dbl_BE_to_system(double input){
				const double litmus = DBL_LITMUS;
				const unsigned char order[8] = LITMUS_AS_MAPPING(((const unsigned char*)&litmus));
				unsigned char* input_as_bytes = (unsigned char *)&input;

				if(LITMUS_UINT64(input_as_bytes) == LITMUS_UINT_BE){
					return input;
				}else if(LITMUS_UINT64(input_as_bytes) == LITMUS_UINT_LE){
					return dbl_bytes_reverse(input);
				}


				unsigned char inverse[8];
				for(int i=0; i<8; i++){
					inverse[order[i]] = i;
				}
				double result;
				unsigned char* bytes = (unsigned char *)&result;
				for(int i=0; i<8; i++){
					(bytes)[i] = input_as_bytes[inverse[i]];
				}
				return result;

			}

			double dbl_LE_to_system(double input){
				const double litmus = DBL_LITMUS;
				const unsigned char order[8] = LITMUS_AS_MAPPING(((const unsigned char*)&litmus));
				unsigned char* input_as_bytes = (unsigned char *)&input;

				if(LITMUS_UINT64(input_as_bytes) == LITMUS_UINT_BE){
					return dbl_bytes_reverse(input);
				}else if(LITMUS_UINT64(input_as_bytes) == LITMUS_UINT_LE){
					return input;
				}

				unsigned char inverse[8];
				for(int i=0; i<8; i++){
					inverse[order[i]] = i;
				}
				double result;
				unsigned char* bytes = (unsigned char *)&result;
				for(int i=0; i<8; i++){
					(bytes)[i] = input_as_bytes[7 - inverse[i]];
				}
				return result;

			}


	#endif	// endianess not known via predefined macros

#endif // std uint64 available



// undefining endianess decoder helper defines so the user namespace ain't pulluted with garbage
#ifdef GALBA_DED_U64_SUPPORTED
	#undef GALBA_DED_U64_SUPPORTED
#endif

#ifdef GALBA_DED_GGC_BSWAP64 
	#undef GALBA_DED_GGC_BSWAP64 
#endif

#ifdef GALBA_DED_GGC_LITTLE
	#undef GALBA_DED_GGC_LITTLE
#endif 

#ifdef GALBA_DED_GGC_BIG
	#undef GALBA_DED_GGC_BIG
#endif

#ifdef DBL_LITMUS
	#undef DBL_LITMUS
#endif

#ifdef LITMUS_AS_MAPPING
	#undef LITMUS_AS_MAPPING
#endif

#ifdef LITMUS_UINT64
	#undef LITMUS_UINT64
#endif

#ifdef UINT64_BYTES_REVERSE
	#undef UINT64_BYTES_REVERSE
#endif

#ifdef LITMUS_UINT_BE
	#undef LITMUS_UINT_BE
#endif

#ifdef LITMUS_UINT_LE
	#undef LITMUS_UINT_LE
#endif



#endif  //GALBA_DOUBLE_ENDIANESS_DECODER    (header guard)