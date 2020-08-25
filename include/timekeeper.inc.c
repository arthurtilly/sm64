#define NUM_CFBs	3
#define STACKSIZE  0x2000
#define  PCM_CHANNELS        2	/* Number of channels */
#define  PCM_CHANNELS_SHIFT  1	/* log2(PCM_CHANNELS) */
#define  PCM_ALIGN           2	/* Alignment of number of samples to send */
#define  PCM_BYTES_PER_SAMPLE  (2 * PCM_CHANNELS) /* Number of bytes in one sample */
#define  PCM_BYTES_PER_SAMPLE_SHIFT  2	/* log2(PCM_BYTES_PER_SAMPLE) */

/*
 * Audio record definitions
 */
#define  AUDIO_SAMPLE_BITS	4
#define  AUDIO_SAMPLES_MAX	(((AUDIO_RECORD_SIZE_MAX-sizeof(HVQM2Audio))*8/AUDIO_SAMPLE_BITS)+1) /* Maximum number of records per sample */

/*
 * PCM buffer specifications
 */
#define  NUM_PCMBUFs	3	/* Number of PCM buffers (2 or more, at least 3 recommended) */
#define  PCMBUF_SPREAD	((PCM_ALIGN-1)+AUDIO_SAMPLES_MAX) /* Minimum required capacity for PCM buffer = Number of samples carried forward from last time + number of samples newly decoded */
#define  PCMBUF_ALIGNED  ((PCMBUF_SPREAD+(PCM_ALIGN-1))&(~(PCM_ALIGN-1))) /* pcmbuf[i] address is aligned */
#define  PCMBUF_SIZE     (PCMBUF_ALIGNED*PCM_CHANNELS)

/*
 * Thread ID and priority
 */
#define IDLE_THREAD_ID         1
#define MAIN_THREAD_ID         3
#define HVQM_THREAD_ID         7
#define TIMEKEEPER_THREAD_ID   8
#define DA_COUNTER_THREAD_ID   9

#define IDLE_PRIORITY         10
#define MAIN_PRIORITY         10
#define HVQM_PRIORITY         11
#define TIMEKEEPER_PRIORITY   12
#define DA_COUNTER_PRIORITY   13

#define PI_COMMAND_QUEUE_SIZE	4

/*
 * Frame buffer state flag 
 */
#define CFB_FREE     0		/* Available */
#define CFB_PRECIOUS (1<<0)	/* Constrained for decoding of next frame */
#define CFB_SHOWING  (1<<1)	/* Waiting to display or displaying */

typedef u32 (*tkAudioProc)(void *pcmbuf);
typedef tkAudioProc (*tkRewindProc)(void);

/***********************************************************************
 * Timekeeper thread
 ***********************************************************************/
static OSThread  tkThread;
static u64  tkStack[STACKSIZE/sizeof(u64)];

/***********************************************************************
 * Audio DA counter thread
 ***********************************************************************/
static OSThread  daCounterThread;
static u64  daCounterStack[STACKSIZE/sizeof(u64)];

/***********************************************************************
 * Timekeeper's command message queue
 ***********************************************************************/
static OSMesgQueue   tkCmdMesgQ;
static OSMesg        tkCmdMesgBuf;

/***********************************************************************
 * Message queue for responses from timekeeper
 ***********************************************************************/
static OSMesgQueue   tkResMesgQ;
static OSMesg        tkResMesgBuf;

/***********************************************************************
 * Timekeeper's command structure
 ***********************************************************************/
typedef struct {
  tkRewindProc rewind;		/* Pointer to stream rewind function */
  u32 samples_per_sec;		/* Audio sampling rate */
} TKCMD;

/***********************************************************************
 * VI event (retrace) message queue
 ***********************************************************************/
#define  VI_MSG_SIZE  2
static OSMesgQueue    viMessageQ;
static OSMesg         viMessages[VI_MSG_SIZE];

/***********************************************************************
 * AI event (FIFO free) message queue
 ***********************************************************************/
#define  AI_MSG_SIZE  2
static OSMesgQueue    aiMessageQ;
static OSMesg         aiMessages[AI_MSG_SIZE];

/***********************************************************************
 * Ring buffer for reserving display of image
 *
 *     As soon as image (frame buffer) is complete, put into this 
 *     ring buffer, wait for display time to come and set in VI.
 ***********************************************************************/
#define  VIDEO_RING_SIZE  NUM_CFBs

static struct {
  u64 disptime;			/* Display time */
  void *vaddr;			/* Frame buffer address */
  u32 *statP;			/* Pointer to frame buffer state flag */
} videoRing[VIDEO_RING_SIZE];

static int  videoRingCount;	/* Amount of data remaining in ring buffer */
static int  videoRingWrite;	/* Ring buffer write index */
static int  videoRingRead;	/* Ring buffer read index */

/***********************************************************************
 * Ring buffer for reserving playback of audio
 * 
 *     As soon as audio (PCM data) is complete, put into this
 *     ring buffer and push as soon as AI FIFO is free.
 ***********************************************************************/
#define  AUDIO_RING_BUFFER_SIZE  NUM_PCMBUFs

static struct {
  void *buf;			/* PCM data buffer */
  u32 len;			/* PCM data length */
} audioRing[AUDIO_RING_BUFFER_SIZE];

static int  audioRingCount;	/* Amount of data remaining in ring buffer */
static int  audioRingWrite;	/* Ring buffer write index */
static int  audioRingRead;	/* Ring buffer read index */

/***********************************************************************
 * PCM data buffer information
 ***********************************************************************/
static int  pcmBufferCount;	/* Number of PCM data buffers being used */
static u32  aiFIFOsamples;	/* Number of PCM samples waiting in AI FIFO */
static u32  aiDAsamples;	/* Number of samples AI is DMA transferring to DA */

/***********************************************************************
 * Buffer for matching PCM transfer data length to PCM_ALIGN 
 ***********************************************************************/
static s16 pcmModBuf[(PCM_ALIGN-1)*PCM_CHANNELS];

/***********************************************************************
 * Flag enabling measurement of passage of time
 ***********************************************************************/
static int  clock_alive;

/***********************************************************************
 * Audio sampling rate
 ***********************************************************************/
static u32  samples_per_sec;

/***********************************************************************
 * Number of PCM samples that have finished playing
 *
 *    Strictly the number samples that have finished playing.  Does not
 *    include samples in FIFO nor the sample playing in the DA buffer.
 *
 *    From this value you get the amount of time that has passed
 *    since the start of play of the HVQM2 stream.
 ***********************************************************************/
static u64  samples_played;

/***********************************************************************
 * The time samples_played was first updated ( osGetTime() )
 *
 *    Time measured beyond this point is supplemented with osGetTime().
 ***********************************************************************/
static OSTime  last_time;

#define OS_CYCLES_TO_USEC(cycles) cycles * 0.00000002133

u64 tkGetTime(void) {
  u64 retval;

  if (!clock_alive) return 0;

  retval = OS_CYCLES_TO_USEC(osGetTime() - last_time);
  if (samples_per_sec) retval += samples_played * 1000000LL / samples_per_sec;
  return retval;
}

static void tkClockDisable(void) {
  clock_alive = 0;
}

static void tkClockStart(void) {
  samples_played = 0;
  last_time = osGetTime();
  clock_alive = 1;
}

static void daCounterProc(void *argument) {
  while (1) {
    osRecvMesg( &aiMessageQ, NULL, OS_MESG_BLOCK );
    last_time = osGetTime();
    if (aiDAsamples > 0) --pcmBufferCount;
    samples_played += aiDAsamples;
    aiDAsamples = aiFIFOsamples;
    aiFIFOsamples = 0;
  }
}

static void timekeeperProc(void *argument) {
    TKCMD *cmd;
    void *showing_cfb = NULL;	/* Frame buffer being displayed */
    u32 *showing_cfb_statP = NULL; /* Pointer to state flag of the frame buffer being displayed */
    
    osCreateMesgQueue( &viMessageQ, viMessages, VI_MSG_SIZE );
    //!osViSetEvent( &viMessageQ, 0, 1 );

    /* Acquire AI event*/
    osCreateMesgQueue( &aiMessageQ, aiMessages, AI_MSG_SIZE );
    osSetEventMesg( OS_EVENT_AI, &aiMessageQ, (OSMesg *)1 );

  /*
   * Create and start audio DA counter thread
   */
    osCreateThread( &daCounterThread, DA_COUNTER_THREAD_ID, daCounterProc, 
		 NULL, daCounterStack + (STACKSIZE/sizeof(u64)), 
		 (OSPri)DA_COUNTER_PRIORITY );
    osStartThread( &daCounterThread );

    osRecvMesg( &tkCmdMesgQ, (OSMesg *)&cmd, OS_MESG_BLOCK );
  
    for ( ; ; ) {
        s32 video_started;		/* Flag indicating start of video display */
        s32 audio_started;		/* Flag indicating start of audio playback */
        s32 video_done;		/* Flag indicating end of video decoding */
        s32 audio_done;		/* Flag indicating end of audio decoding */
        void *pushed_cfb;		/* Frame buffer reserved for swap */
        u32 *pushed_cfb_statP;	/* Pointer to state flag of frame buffer reserved for swap */
        s32 next_pcmbufno;		/* The number of the PCM buffer to use next */
        s32 pcm_mod_samples;	/* The number of PCM samples left over */
        tkAudioProc audioproc;
        OSTime present_vtime;
        
        //while ( cmd == NULL ) {
            osSendMesg( &tkResMesgQ, NULL, OS_MESG_BLOCK );
            osRecvMesg( &tkCmdMesgQ, (OSMesg *)&cmd, OS_MESG_BLOCK );
        //}
    
        tkClockDisable();
        audioproc = (cmd->rewind)();
    }
}

void createTimekeeper(void) {
  osCreateMesgQueue( &tkCmdMesgQ, &tkCmdMesgBuf, 1 );
  osCreateMesgQueue( &tkResMesgQ, &tkResMesgBuf, 1 );
  osCreateThread( &tkThread, TIMEKEEPER_THREAD_ID, timekeeperProc, 
		 NULL, tkStack + (STACKSIZE/sizeof(u64)), 
		 (OSPri)TIMEKEEPER_PRIORITY );
  osStartThread( &tkThread );
}

void tkStart( tkRewindProc rewind, u32 samples_per_sec ) {
  TKCMD tkcmd;
  tkcmd.samples_per_sec = samples_per_sec;
  tkcmd.rewind = rewind;
  osSendMesg( &tkCmdMesgQ, (OSMesg *)&tkcmd, OS_MESG_BLOCK );
  osRecvMesg( &tkResMesgQ, (OSMesg *)NULL, OS_MESG_BLOCK );
}

void tkStop( void ) {
  osSendMesg( &tkCmdMesgQ, (OSMesg *)NULL, OS_MESG_BLOCK );
  osRecvMesg( &tkResMesgQ, (OSMesg *)NULL, OS_MESG_BLOCK );
}

void tkPushVideoframe( void *vaddr, u32 *statP, u64 disptime ) {
  *statP |= CFB_SHOWING;
  while ( videoRingCount >= VIDEO_RING_SIZE ) func_802F71F0();
  videoRing[videoRingWrite].disptime = disptime;
  videoRing[videoRingWrite].vaddr = vaddr;
  videoRing[videoRingWrite].statP = statP;
  if ( ++videoRingWrite == VIDEO_RING_SIZE ) videoRingWrite = 0;
  ++videoRingCount;
}
