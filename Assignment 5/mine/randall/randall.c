/* Generate N bytes of random output.  */

/* When generating output this program uses the x86-64 RDRAND
   instruction if available to generate random numbers, falling back
   on /dev/random and stdio otherwise.

   This program is not portable.  Compile it with gcc -mrdrnd for a
   x86-64 machine.

   Copyright 2015, 2017, 2020 Paul Eggert

   This program is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <cpuid.h>
#include <errno.h>
#include <immintrin.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "./output.h"
#include "./rand64-hw.h"
#include "./rand64-sw.h"
#include "./options.h"
// /* Hardware implementation.  */

// /* Description of the current CPU.  */
// struct cpuid { unsigned eax, ebx, ecx, edx; };

// /* Return information about the CPU.  See <http://wiki.osdev.org/CPUID>.  */
// static struct cpuid
// cpuid (unsigned int leaf, unsigned int subleaf)
// {
//   struct cpuid result;
//   asm ("cpuid"
//        : "=a" (result.eax), "=b" (result.ebx),
// 	 "=c" (result.ecx), "=d" (result.edx)
//        : "a" (leaf), "c" (subleaf));
//   return result;
// }

// /* Return true if the CPU supports the RDRAND instruction.  */
// static _Bool
// rdrand_supported (void)
// {
//   struct cpuid extended = cpuid (1, 0);
//   return (extended.ecx & bit_RDRND) != 0;
// }

// /* Initialize the hardware rand64 implementation.  */
// static void
// hardware_rand64_init (void)
// {
// }

// /* Return a random value, using hardware operations.  */
// static unsigned long long
// hardware_rand64 (void)
// {
//   unsigned long long int x;
//   while (! _rdrand64_step (&x))
//     continue;
//   return x;
// }

// /* Finalize the hardware rand64 implementation.  */
// static void
// hardware_rand64_fini (void)
// {
// }



// /* Software implementation.  */

// /* Input stream containing random bytes.  */
// static FILE *urandstream;

// /* Initialize the software rand64 implementation.  */
// static void
// software_rand64_init (void)
// {
//   urandstream = fopen ("/dev/random", "r");
//   if (! urandstream)
//     abort ();
// }

// /* Return a random value, using software operations.  */
// static unsigned long long
// software_rand64 (void)
// {
//   unsigned long long int x;
//   if (fread (&x, sizeof x, 1, urandstream) != 1)
//     abort ();
//   return x;
// }

// /* Finalize the software rand64 implementation.  */
// static void
// software_rand64_fini (void)
// {
//   fclose (urandstream);
// }

/*
static bool
writebytes (unsigned long long x, int nbytes)
{
  do
    {
      if (putchar (x) < 0)
	return false;
      x >>= CHAR_BIT;
      nbytes--;
    }
  while (0 < nbytes);

  return true;
}
*/
FILE *urandstream2;
char *input_file;

void 
file_init (void) 
{
  urandstream2 = fopen (input_file, "r");
    if (! urandstream2) {
      fprintf (stderr, "Error: File could not be opened.");
      exit(1);
    }
}

unsigned long long
file_rand64 (void)
{
  unsigned long long int x;
  if (fread (&x, sizeof x, 1, urandstream2) != 1)
    abort ();
  return x;
}

void
file_rand64_fini (void)
{
  fclose (urandstream2);
}



/* Main program, which outputs N bytes of random data.  */
int
main (int argc, char **argv)
{
  void (*initialize) (void);
  unsigned long long (*rand64) (void);
  void (*finalize) (void);



  struct opts Options;
  // Options.input_flag = 0;
  // Options.output_flag = 0;
  Options.output=0;
  read_options(argc, argv, &Options);

  
  // printf("%d: valid\n", Options.valid);
  // printf("%llu: nbytes\n", Options.nbytes);
  // printf("%d: input\n", Options.input);
  // printf("%s: r_src\n", Options.r_src);
  // printf("%d: output\n", Options.output);
  // printf("%d: blocksize\n", Options.block_size);




  bool valid = Options.valid;
  long long nbytes = Options.nbytes;

  
    if (Options.input == RDRAND){
      if (rdrand_supported ())
      {
        initialize = hardware_rand64_init;
        rand64 = hardware_rand64;
        finalize = hardware_rand64_fini;
      }

      else {
        fprintf (stderr, "Error: Hardware not supported.");
        exit(1);
      }
    }

    else if (Options.input == MRAND48_R) {
      initialize = software_rand64_init;
      rand64 = software_rand64;
      finalize = software_rand64_fini;
    }
  
    else if (Options.input == SLASH_F) {
      input_file = Options.r_src;
      initialize = file_init;
      rand64 = file_rand64;
      finalize = file_rand64_fini;    
    }

    else{
      fprintf (stderr, "Error: Option is not supported.");
      exit(1);
    }
  




  initialize ();

  //options for output
  
    int output_errno = 0;
    if (Options.output == STDOUT) {
      int wordsize = sizeof rand64 ();
      
      do
      {
        unsigned long long x = rand64 ();
        int outbytes = nbytes < wordsize ? nbytes : wordsize;
        if (!writebytes (x, outbytes))
      {
      output_errno = errno;
      break;
      }
        nbytes -= outbytes;
      }
      while (0 < nbytes);
    }

    else if(Options.output==1 && Options.block_size==0) {
      fprintf (stderr, "Error: Provided option is not supported.");
      exit(1);
    }

    else if(Options.output==1 && Options.nbytes==0) {
      fprintf (stderr, "Error: Provided option is not supported.");
      exit(1);
    }

    else if (Options.output == N) {
      int size = Options.block_size * 1024;
      unsigned long long *buffer = malloc(size);
      if (buffer == NULL){
        fprintf (stderr, "Error: Not enough space to allocate buffer.");
        exit(1);
      }

      do {
        int outbytes = nbytes < size ? nbytes : size;
        int times = size / sizeof rand64();
        for (int i = 0; i < times; i++){
          unsigned long long x = rand64 ();
          buffer[i] = x;
        }
        int temp = write(1, buffer, outbytes);
        if (temp < 0) {
          fprintf (stderr, "Error: Output could not be printed.");
          free(buffer);
          exit(1);
        }
        nbytes -= temp;
      }
      while (0 < nbytes);
      free(buffer);
    }

    else{
      fprintf (stderr, "Error: Option is not supported.");
      exit(1);
    }
  

 
  
  /* Check arguments.  */
  //bool valid = false;
  //long long nbytes;
  // if (argc == 2)
  //   {
  //     char *endptr;
  //     errno = 0;
  //     nbytes = strtoll (argv[1], &endptr, 10);
  //     if (errno)
	// perror (argv[1]);
  //     else
	// valid = !*endptr && 0 <= nbytes;
  //   }
 if (!valid)
    {
     fprintf (stderr, "%s: usage: %s NBYTES\n", argv[0], argv[0]);
     return 1;
    }

  /* If there's no work to do, don't worry about which library to use.  */
  if (nbytes == 0)
    return 0;

  /* Now that we know we have work to do, arrange to use the
     appropriate library.  */
  // void (*initialize) (void);
  // unsigned long long (*rand64) (void);
  // void (*finalize) (void);

  /*
  if (rdrand_supported ())
    {
      initialize = hardware_rand64_init;
      rand64 = hardware_rand64;
      finalize = hardware_rand64_fini;
    }
  else
    {
      initialize = software_rand64_init;
      rand64 = software_rand64;
      finalize = software_rand64_fini;
    }

  if (Options.input != SLASH_F){
    initialize ();
  }
  */

  /*
  int wordsize = sizeof rand64 ();

  do
    {
      unsigned long long x = rand64 ();
      int outbytes = nbytes < wordsize ? nbytes : wordsize;
      if (!writebytes (x, outbytes))
	{
	  output_errno = errno;
	  break;
	}
      nbytes -= outbytes;
    }
  while (0 < nbytes);
    */

  if (fclose (stdout) != 0)
    output_errno = errno;

  if (output_errno)
    {
      errno = output_errno;
      perror ("output");
    }

  finalize ();
  return !!output_errno;
}
