/* This is from http://tldp.org/HOWTO/MIDI-HOWTO-9.html#ss9.2

   ALSA Sequencer MIDI redirector.
   Redirects the input to outputs determined by the MIDI channel
   (as requested by Nathaniel Virgo on Linux-Audio-Dev ;-)
   based on Dr. Matthias Nagorni's ALSA seq example
      
   Nick Dowell <nixx@nixx.org.uk>
   */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>

int
main()
{
  snd_seq_t *seq_handle;
  snd_seq_event_t *ev;
  int i;
  int portid;              /* input port */
  int oportid[16];         /* output ports */
  int npfd;
  struct pollfd *pfd;
  char txt[20];

  if (snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }

  snd_seq_set_client_name(seq_handle, "MIDI Redirect");
  
  /* open one input port */
  if ((portid = snd_seq_create_simple_port
       (seq_handle, "Input",
        SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "fatal error: could not open input port.\n");
    exit(1);
  }
  /* open 16 output ports for the MIDI channels */
  for (i=0; i<16; i++){
    sprintf( txt, "MIDI Channel %d", i );
    if ((oportid[i] = snd_seq_create_simple_port
         (seq_handle, txt,
          SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
          SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
      fprintf(stderr, "fatal error: could not open output port.\n");
      exit(1);
    }
  }

  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);

  while (1)  /* main loop */
    if (poll(pfd, npfd, 1000000) > 0){
      do {
        snd_seq_event_input(seq_handle, &ev);
        snd_seq_ev_set_source( ev, oportid[ev->data.control.channel] );
        snd_seq_ev_set_subs( ev );
        snd_seq_ev_set_direct( ev );
        snd_seq_event_output_direct( seq_handle, ev );
        snd_seq_free_event(ev);
      } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
    }
  return 0;
}
