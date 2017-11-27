// Taken from https://github.com/jeschke/water-wave-packets
// Originally GlobalDefs.h

// Global definitions needed for packet simulation and rendering

// scene parameters
#define SCENE_EXTENT 100.0f					// extent of the entire scene (packets traveling outside are removed)
#define MIN_WATER_DEPTH 0.1f				// minimum water depth (meters)
#define MAX_WATER_DEPTH 5.0f				// maximum water depth (meters)
#define WATER_TERRAIN_FILE "./textures/TestIsland.bmp"// Contains water depth and land height in different channels


// rendering parameters
#define PACKET_GPU_BUFFER_SIZE 1000000		// maximum number of wave packets to be displayed in one draw call


/*
// Fast rendering setup
#define WAVETEX_WIDTH_FACTOR 0.5	// the wavemesh texture compared to screen resolution
#define WAVETEX_HEIGHT_FACTOR 1		// the wavemesh texture compared to screen resolution
#define WAVEMESH_WIDTH_FACTOR 0.1	// the fine wave mesh compared to screen resolution
#define WAVEMESH_HEIGHT_FACTOR 0.25	// the fine wave mesh compared to screen resolution
#define AA_OVERSAMPLE_FACTOR 2		// anti aliasing applied in BOTH X and Y directions  {1,2,4,8}
*/

/*
// Balanced rendering  setup
#define WAVETEX_WIDTH_FACTOR 1	    // the wavemesh texture compared to screen resolution
#define WAVETEX_HEIGHT_FACTOR 2	    // the wavemesh texture compared to screen resolution
#define WAVEMESH_WIDTH_FACTOR 1		// the fine wave mesh compared to screen resolution
#define WAVEMESH_HEIGHT_FACTOR 2	// the fine wave mesh compared to screen resolution
#define AA_OVERSAMPLE_FACTOR 2		// anti aliasing applied in BOTH X and Y directions  {1,2,4,8}
*/


// High quality rendering  setup
#define WAVETEX_WIDTH_FACTOR 2		// the wavemesh texture compared to screen resolution
#define WAVETEX_HEIGHT_FACTOR 4		// the wavemesh texture compared to screen resolution
#define WAVEMESH_WIDTH_FACTOR 2		// the fine wave mesh compared to screen resolution
#define WAVEMESH_HEIGHT_FACTOR 4	// the fine wave mesh compared to screen resolution
#define AA_OVERSAMPLE_FACTOR 4		// anti aliasing applied in BOTH X and Y directions  {1,2,4,8}

// UI consts
#define MAX_PACKET_BUDGET 100000
#define MIN_PACKET_BUDGET 1000
#define MIN_WAVE_SPEED 0.0
#define MAX_WAVE_SPEED 1.0
#define INIT_WAVE_SPEED 0.07
