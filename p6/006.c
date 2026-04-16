#include "src/go.h"
#include <stdio.h>
#include <stdlib.h>

#define N (2 << 15)

Value send_efficient() {
  Channel *ch_efficient = receive(me()).asChannel;
  send(ch_efficient, receive(me()));
  send(me(), asInt(1));
  return asInt(0);
}

Value pop_efficient() {
  Channel *ch_efficient = receive(me()).asChannel;
  send(me(), receive(ch_efficient));
  return asInt(0);
}

int og_argc;
char *og_argv0;
char *og_envp0;
int main_cnt = 0;

int main(int argc, char *argv[], char *envp[]) {
  /* TEST: again on main with all arguments
   * TIP: saving these arguments at the start
   */
  if (main_cnt == 0) {
    og_argc = argc;
    og_argv0 = argv[0];
    og_envp0 = envp[0];
    main_cnt = 1;
    again();
  } else {
    bool argc_pass = og_argc == argc;
    bool argv0_pass = og_argv0 == argv[0];
    bool envp_pass = og_envp0 == envp[0];
    if (!argc_pass || !argv0_pass || !envp_pass) {
      printf("Main again failed!\n\t argc_pass: %d, argv0_pass: %d, envp_pass: "
             "%d\n",
             argc_pass, argv0_pass, envp_pass);
      return 1;
    }
  }

  /* TEST: Channel queues are efficient
   * TIP: Try using a channel queue implemented with a linked list for O(1) inserts and
   *      polls. This test also requires your main stack to be infinite after again in main 
   */
  Channel *snd_chnls[N];
  Channel *rcvr_chnls[N];
  Channel *ch_efficient = channel();

  // Send 
  for (long i = 0; i < N; i++) {
    snd_chnls[i] = go(send_efficient);
    send(snd_chnls[i], asChannel(ch_efficient));
    send(snd_chnls[i], asLong(i));
  }

  // Pop them
  for (long i = 0; i < N; i++) {
    rcvr_chnls[i] = go(pop_efficient);
    send(rcvr_chnls[i], asChannel(ch_efficient));
  }

  // Wait to finish
  for (long i = 0; i < N; i++) {
    receive(snd_chnls[i]);
  }
  for (long i = 0; i < N; i++) {
    receive(rcvr_chnls[i]);
  }

  printf("Good job! Get some sleep please :)\n");
  return 0;
}
