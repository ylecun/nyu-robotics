/* jackMiniSynth 1.0 by Matthias Nagorni    */
/* The Moog filter has been taken from      */
/* http://musicdsp.org.                     */              
/* jackMiniSynth is licensed under the GPL. */

#include <stdio.h>   
#include <stdlib.h>
#include <signal.h>
#include <jack/jack.h>
#include <alsa/asoundlib.h>
#include <math.h>

#define MAXPOLY 64
#define MIN_FREQ                         20
#define MAX_FREQ                      22000

int signal_count;
int midi_channel, poly;
snd_seq_t *seq_handle;
unsigned long rate;
jack_client_t *jack_handle;
jack_port_t *output_port;
char client_name[256];
double gain, resonance, cutoff, sweep, phi[MAXPOLY], phi_mod[MAXPOLY], pitch, velocity[MAXPOLY];
double attack, decay, sustain, release, f_attack, f_decay, f_sustain, f_release, f_env_amount;
double env_time[MAXPOLY], env_level[MAXPOLY], f_env_level[MAXPOLY];
int transpose, note[MAXPOLY], gate[MAXPOLY], note_active[MAXPOLY];
double moog_in[5][MAXPOLY], moog_buf[5][MAXPOLY], filter_freq[MAXPOLY];

void sigterm_exit(int sig) {

   signal_count++;
   if (signal_count < 2) {
     printf("Closing JACK client.\n");
     jack_client_close (jack_handle);
     snd_seq_close (seq_handle);
   } else {
     printf("JACK client closed.\n");
   }
   exit(0);
}

double envelope(int *note_active, int gate, double *env_level, double t, double attack, double decay, double sustain, double release) {

    if (gate)  {
        if (t > attack + decay) return(*env_level = sustain);
        if (t > attack) return(*env_level = 1.0 - (1.0 - sustain) * (t - attack) / decay);
        return(*env_level = t / attack);
    } else {
        if (t > release) {
            if (note_active) *note_active = 0;
            return(*env_level = 0);
        }
        return(*env_level * (1.0 - t / release));
    }
}

double f_envelope(int gate, double *f_env_level, double t, double attack, double decay, double sustain, double release) {

    if (gate)  {
        if (t > attack + decay) return(*f_env_level = sustain);
        if (t > attack) return(*f_env_level = 1.0 - (1.0 - sustain) * (t - attack) / decay);
        return(*f_env_level = t / attack);
    } else {
        if (t > release) {
          return(*f_env_level = 0);
        }
        return(*f_env_level * (1.0 - t / release));
    }
}

int midi_callback() {

    snd_seq_event_t *ev;
    int l1, interval, new_note, next_free_voice;

    do {
        snd_seq_event_input(seq_handle, &ev);
        if (midi_channel < 0) {
          if ((ev->type == SND_SEQ_EVENT_NOTEON) && (ev->data.note.velocity == 0)) 
            ev->type = SND_SEQ_EVENT_NOTEOFF;        
        } else {
          if ((ev->type == SND_SEQ_EVENT_NOTEON) && (ev->data.note.velocity == 0) 
           && (ev->data.control.channel == midi_channel)) 
            ev->type = SND_SEQ_EVENT_NOTEOFF;        
        }
        switch (ev->type) {
            case SND_SEQ_EVENT_PITCHBEND:
                pitch = (double)ev->data.control.value / 8192.0;
                break;
            case SND_SEQ_EVENT_CONTROLLER:
                switch(ev->data.control.param) {
                  case 74:
                    cutoff = 7.0 * (double)ev->data.control.value / 127.0;
                    break;
                  case 100:
                    gain = 0.5 * (double)ev->data.control.value / 127.0;  
                    break;
                  case 3:
                    transpose = (int)(12.0 * (double)ev->data.control.value / 127.0);
                    break;
                  case 71: 
                    resonance = 0.01 + (double)ev->data.control.value / 127.0;  
                    break;
                  case 16: 
                    attack = 0.01 + (double)ev->data.control.value / 127.0;  
                    break;
                  case 80: 
                    decay = 0.01 + (double)ev->data.control.value / 127.0;  
                    break;
                  case 19: 
                    sustain = (double)ev->data.control.value / 127.0;  
                    break;
                  case 2: 
                    release = 0.01 + (double)ev->data.control.value / 127.0;  
                    break;
                  case 9: 
                    f_attack = 0.01 + (double)ev->data.control.value / 127.0;  
                    break;
                  case 10: 
                    f_decay = 0.01 + (double)ev->data.control.value / 127.0;  
                    break;
                  case 11: 
                    f_sustain = (double)ev->data.control.value / 127.0;  
                    break;
                  case 12: 
                    f_release = 0.01 + (double)ev->data.control.value / 127.0;  
                    break;
                  case 81: 
                    f_env_amount = 6.0 - 12.0 * (double)ev->data.control.value / 127.0;  
                    break;
                  case 14: 
                    sweep = 10.0 * (double)ev->data.control.value / 127.0;  
                    break;
                  } 
                break;
            case SND_SEQ_EVENT_NOTEON:
                if ((midi_channel < 0) || (ev->data.control.channel == midi_channel)) {
		  printf("note on chan=%d, note=%d\n",ev->data.control.channel,ev->data.note.note);
                  next_free_voice = 0;
                  interval = 100; 
                  new_note = ev->data.note.note;
                  for (l1 = 0; l1 < poly; l1++) {
                    if (!note_active[l1] && (abs(note[l1] - new_note) < interval)) {
                      next_free_voice = l1;
                      interval = abs(note[l1] - new_note);
                    }
                  }    
                  note[next_free_voice] = new_note;
                  velocity[next_free_voice] = ev->data.note.velocity / 127.0;
                  env_time[next_free_voice] = 0;
                  gate[next_free_voice] = 1;
                  note_active[next_free_voice] = 1;
                }
                break;        
            case SND_SEQ_EVENT_NOTEOFF:
                if ((midi_channel < 0) || (ev->data.control.channel == midi_channel)) {
		  printf("note off\n");
                    for (l1 = 0; l1 < poly; l1++) {
                        if (gate[l1] && note_active[l1] && (note[l1] == ev->data.note.note)) {
                            env_time[l1] = 0;
                            gate[l1] = 0;
                        }
                    }
                }    
                break;        
        }
        snd_seq_free_event(ev);
    } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
    return (0);
}

int playback_callback(jack_nframes_t nframes, void *arg) {

    int l1, l2;
    double filter_gain, log2, freq_tune, f_log, f, inv_rate, inv2_rate, dphi, f1, f2, f3, freq_note, sound;
    double moog_f, fa, fb, q, d, dd, dsaw, wave_period, edge;
      
    wave_period = 65536.0;  
    edge = 1.9;
    filter_gain = 0.3;
    log2 = log(2.0);
    jack_default_audio_sample_t *buf = (jack_default_audio_sample_t *)jack_port_get_buffer(output_port, nframes);  
    memset(buf, 0, nframes * sizeof(jack_default_audio_sample_t));
    for (l2 = 0; l2 < poly; l2++) {
      if (note_active[l2]) {
        f1 = 8.176 * exp((double)(transpose+note[l2]-12)*log(2.0)/12.0);
        f2 = 8.176 * exp((double)(transpose+note[l2])*log(2.0)/12.0);
        f3 = 8.176 * exp((double)(transpose+note[l2]+12)*log(2.0)/12.0);
        freq_note = (pitch > 0) ? f2 + (f3-f2)*pitch : f2 + (f2-f1)*pitch;
        dphi = wave_period * freq_note / (double)rate;
        d = edge * dphi;
        dd = 1.0 / d;
        dsaw = 2.0 / (wave_period - 2.0 * d);
        q = resonance;
        if (q < 0.01) q = 0.01;
        else if (q > 1) q = 1; 
        inv2_rate = 2.0 / (double)rate;
        inv_rate = 1.0 / (double)rate;
        freq_tune = 5.0313842 + cutoff;
        for (l1 = 0; l1 < nframes; l1++) {
          if (phi[l2] <= d) {
            sound = phi[l2] * dd;
          } else {
            if (phi[l2] <= wave_period - d) {
              sound = 1.0 - (phi[l2] - d) * dsaw;
            } else {
              sound = (phi[l2] - wave_period) * dd;
            }
          }
          phi[l2] += dphi;
          while (phi[l2] < 0) phi[l2] += wave_period;
          while (phi[l2] > wave_period) phi[l2] -= wave_period;
          f_log = log2 * (freq_tune + note[l2] / 12.0 + pitch)
                + f_env_amount * f_envelope(gate[l2], &f_env_level[l2], env_time[l2], f_attack, f_decay, f_sustain, f_release);
          filter_freq[l2] -= 0.0001 * sweep * (filter_freq[l2] - f_log);
          f = exp(filter_freq[l2]);
          if (f < MIN_FREQ) f = MIN_FREQ;
          else if (f > MAX_FREQ) f = MAX_FREQ;
          fa = inv2_rate * f;
          moog_f = fa * 1.16;
          fb = 4.1 * q * (1.0 - 0.15 * moog_f * moog_f);
          moog_in[0][l2] = filter_gain * sound;
          moog_in[0][l2] -= fb * moog_buf[4][l2];
          moog_in[0][l2] *= 0.35013 * moog_f * moog_f * moog_f * moog_f; 
          moog_buf[1][l2] = moog_in[0][l2] + 0.3 * moog_in[1][l2] + (1.0 - moog_f) * moog_buf[1][l2];
          moog_in[1][l2] = moog_in[0][l2];
          moog_buf[2][l2] = moog_buf[1][l2] + 0.3 * moog_in[2][l2] + (1.0 - moog_f) * moog_buf[2][l2];
          moog_in[2][l2] = moog_buf[1][l2]; 
          moog_buf[3][l2] = moog_buf[2][l2] + 0.3 * moog_in[3][l2] + (1.0 - moog_f) * moog_buf[3][l2];
          moog_in[3][l2] = moog_buf[2][l2];
          moog_buf[4][l2] = moog_buf[3][l2] + 0.3 * moog_in[4][l2] + (1.0 - moog_f) * moog_buf[4][l2];
          moog_in[4][l2] = moog_buf[3][l2];
          sound = moog_buf[4][l2];
          sound *= gain * envelope(&note_active[l2], gate[l2], &env_level[l2], env_time[l2], attack, decay, sustain, release)
                        * velocity[l2];
          env_time[l2] += inv_rate;
          buf[l1] += sound;
        }
      }    
    }
    return(0);
}

snd_seq_t *open_seq() {

    snd_seq_t *seq_handle;
    int client_id, port_id;
    
    if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0) < 0) {
        fprintf(stderr, "Error opening ALSA sequencer.\n");
        exit(1);
    }
    snd_seq_set_client_name(seq_handle, "jackMiniSynth");
    client_id = snd_seq_client_id(seq_handle);
    if (port_id = snd_seq_create_simple_port(seq_handle, "jackMiniSynth",
        SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_APPLICATION) < 0) {
        fprintf(stderr, "Error creating sequencer port.\n");
        exit(1);
    }
    sprintf(client_name, "jackMiniSynth_%d_%d", client_id, port_id);
    return(seq_handle);
}

jack_client_t *open_jack() {

    jack_client_t *jack_handle;
            
    if ((jack_handle = jack_client_new(client_name)) == 0) {
        fprintf (stderr, "jack server not running ?\n");
        exit (1);
    }
    output_port = jack_port_register(jack_handle, "Out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    jack_set_process_callback(jack_handle, playback_callback, 0);
    rate = jack_get_sample_rate(jack_handle);
    return(jack_handle);
}

int main (int argc, char *argv[]) {

    int seq_nfds, l1, l2;
    struct pollfd *pfds;
    
    if (argc < 16) {
        fprintf(stderr, "jackMiniSynth <MIDI ch> <poly> <gain> <transpose> <resonance> <a> <d> <s> <r> <filt_a> <filt_d> <filt_s> <filt_r> <filt_env_amount> <sweep>\n"); 
        exit(1);
    }
    midi_channel = atoi(argv[1]);
    poly = atoi(argv[2]);
    gain = atof(argv[3]);
    transpose = atoi(argv[4]);
    resonance = atof(argv[5]);
    attack = atof(argv[6]);
    decay = atof(argv[7]);
    sustain = atof(argv[8]);
    release = atof(argv[9]);
    f_attack = atof(argv[10]);
    f_decay = atof(argv[11]);
    f_sustain = atof(argv[12]);
    f_release = atof(argv[13]);
    f_env_amount = atof(argv[14]);
    sweep = atof(argv[15]);
    pitch = 0;
    cutoff= 5;
    seq_handle = open_seq();
    jack_handle = open_jack();
    signal_count = 0;
    signal(SIGINT, sigterm_exit);
    signal(SIGTERM, sigterm_exit);
    seq_nfds = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
    pfds = (struct pollfd *)alloca(sizeof(struct pollfd) * seq_nfds);
    snd_seq_poll_descriptors(seq_handle, pfds, seq_nfds, POLLIN);
    for (l1 = 0; l1 < poly; l1++) {
      note_active[l1] = 0;
      filter_freq[l1] = cutoff;
      for (l2 = 0; l2 < 5; l2++) {
        moog_in[l2][l1] = 0;
        moog_buf[l2][l1] = 0;
      } 
    }
    if (jack_activate(jack_handle)) {
      fprintf(stderr, "cannot activate client");
      exit(1);
    }
    while (1) {
        if (poll (pfds, seq_nfds, 1000) > 0) {
            for (l1 = 0; l1 < seq_nfds; l1++) {
               if (pfds[l1].revents > 0) midi_callback();
            }
        }
    }
    return (0);
}
