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
#include <time.h>



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


struct drand48_data buf = {0};
long int a, b;

void mrand48_rng_init()
{

}

unsigned long long mrand48_rng()
{
    mrand48_r(&buf, &a);
    mrand48_r(&buf, &b);
    unsigned long long int x;
    x = (((unsigned long long) a) << 32) | ((unsigned long long) b & 0x00000000FFFFFFFF);
    return x;
}
void mrand48_rng_fini()
{
    
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
      initialize = mrand48_rng_init;
      rand64 = mrand48_rng;
      finalize = mrand48_rng_fini;
      
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
  
 if (!valid)
    {
     fprintf (stderr, "%s: usage: %s NBYTES\n", argv[0], argv[0]);
     return 1;
    }

  /* If there's no work to do, don't worry about which library to use.  */
  if (nbytes == 0)
    return 0;

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
