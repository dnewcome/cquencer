#include <stdio.h>
#include <stdlib.h>
#include "rtmidi_c.h"

// note on message
unsigned char msg[3] = {0x90, 0x3c, 0x40};

int main()
{
  // enumerate the available apis.. could be core, alsa, jack, etc
  int num_apis = rtmidi_get_compiled_api(NULL, 0);
  printf("we have %i apis\n", num_apis);
  enum RtMidiApi* apis = malloc(num_apis * sizeof(enum RtMidiApi));
  rtmidi_get_compiled_api(apis, num_apis);
  // name of first api is `core' on osx
  printf("api name is %s\n", rtmidi_api_name(apis[0]));

  // open a port 
  RtMidiOutPtr out_ptr = rtmidi_out_create(apis[0], "test");
  unsigned int num_ports = rtmidi_get_port_count(out_ptr);
  printf("we have %i ports\n", num_ports);
  rtmidi_open_port(out_ptr, 0, "Bus 1");

  // send a message to the port
  int retval = rtmidi_out_send_message(out_ptr, msg , 3);
  printf("message sent, return  %i\n", retval);

  return 0;
}
