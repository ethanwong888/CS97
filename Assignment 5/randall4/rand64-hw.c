#include <cpuid.h>
#include <immintrin.h>
#include <time.h>
#include "./rand64-hw.h"

/* Hardware implementation.  */

struct cpuid { unsigned eax, ebx, ecx, edx; };

/* Return information about the CPU.  See <http://wiki.osdev.org/CPUID>.  */
struct cpuid
cpuid (unsigned int leaf, unsigned int subleaf)
{
  struct cpuid result;
  asm ("cpuid"
       : "=a" (result.eax), "=b" (result.ebx),
	 "=c" (result.ecx), "=d" (result.edx)
       : "a" (leaf), "c" (subleaf));
  return result;
}

/* Return true if the CPU supports the RDRAND instruction.  */
_Bool
rdrand_supported (void)
{
  struct cpuid extended = cpuid (1, 0);
  return (extended.ecx & bit_RDRND) != 0;
}

// struct drand48_data drand_buf;

/* Initialize the hardware rand64 implementation.  */
void
hardware_rand64_init (void)
{
  // srand48_r(time(NULL), &drand_buf);
  // long int result;
}

/* Return a random value, using hardware operations.  */
unsigned long long
hardware_rand64 (void)
{
  unsigned long long int x;
  // mrand48_r(&drand_buf, &result);
  while (! _rdrand64_step (&x))
    continue;
  return x;
}

/* Finalize the hardware rand64 implementation.  */
void
hardware_rand64_fini (void)
{
}
