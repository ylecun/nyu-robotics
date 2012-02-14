#include <jack/jack.h>

#include <signal.h>
#include <unistd.h>
#include <stdio.h>

jack_client_t *client;
jack_port_t   *port;

/* this is the heart of the client. the process callback. 
 * this will be called by jack every process cycle.
 * jack provides us with a buffer for out output port, 
 * which we can happily write into. inthis case we just 
 * fill it with 0's to produce.... silence! not to bad, eh? */
int process(jack_nframes_t nframes, void *arg) {
	int index;
	
	/* this function returns a pointer to the buffer where 
     * we can write our frames samples */
	float *buffer = jack_port_get_buffer(port, nframes);

	/* so we do it :) */
	for (index = 0; index < nframes; ++index) {
		buffer[index] = 0;
	}
	
	return 1;
}

/* a flag which will be set by our signal handler when 
 * it's time to exit */
int quit = 0;

/* the signal handler */
void signalled(int signal) {
	quit = 1;
}

int main() {
	/* setup our signal handler signalled() above, so 
	 * we can exit cleanly (see end of main()) */
	signal(SIGINT, signalled);


	/* naturally we need to become a jack client :) */
	client = jack_client_new("foobar");
	if (!client) {
		printf("couldn't connect to jack server. Either it's not running or the client name is already taken\n");
		exit(1);
	}

	/* we register an output port and tell jack it's a 
	 * terminal port which means we don't 
	 * have any input ports from which we could somhow 
	 * feed our output */
	port = jack_port_register(client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput|JackPortIsTerminal, 0);

	/* jack is callback based. That means we register 
	 * a callback function (see process() above)
	 * which will then get called by jack once per process cycle */
	jack_set_process_callback(client, process, 0);

	/* tell jack that we are ready to do our thing */
	jack_activate(client);
	
	/* wait until this app receives a SIGINT (i.e. press 
	 * ctrl-c in the terminal) see signalled() above */
	while(!quit) 
		/* let's not waste cycles by busy waiting */
		sleep(1);
	
	/* so we shall quit, eh? ok, cleanup time. otherwise 
	 * jack would probably produce an xrun
	 * on shutdown */
	jack_deactivate(client);

	/* shutdown cont. */
	jack_client_close(client);

	/* done !! */
	return 0;
}
