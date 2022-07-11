/* Generate sizeof(uint32_t) bytes of as random data as possible to seed
   the hash function.
*/

#include <stdio.h>
#include <stdint.h>
#include "jansson.h"



static uint32_t generate_seed(void)
{
  uint32_t seed = 0;


  seed = rand();

  /* Make sure the seed is never zero */
  if (seed == 0) seed = 1;

  return seed;
}


volatile uint32_t hashtable_seed = 0;

/* Fall back to a thread-unsafe version */
void json_object_seed(size_t seed)
{
  uint32_t new_seed = (uint32_t)seed;

  if (hashtable_seed == 0)
  {
    if (new_seed == 0)
    new_seed = generate_seed();

    hashtable_seed = new_seed;
  }
}

