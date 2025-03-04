/* ReC98
 * -----
 * Code segment #3 of TH02's MAIN.EXE
 */

extern "C" {
#include <stddef.h>
#include "platform.h"
#include "pc98.h"
#include "planar.h"
#include "th02/math/randring.h"

#define RANDRING_INSTANCE 2
#include "th02/math/randring.cpp"

#include "th02/main/bullet/pellet_r.cpp"
}
