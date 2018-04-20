/* sock_reuserps.h

  Faruk Grozdanic github/g5unit
  
  Released under GPLv3.0
*/

#include <linux/types.h>
#include <net/sock.h>
#include <linux/socket.h>

int sorps_add_listener(struct sock* sk, struct socket* fd, int cpuid);
int sorps_decrement_listener(struct sock* sk, struct socket* fd, int cpuid);
int sorps_remove_listener(struct socket* fd, int cpuid);
struct socket* sorps_lookup_listener(struct sock* sk, int cpuid);
