#define PF_FN_LEN 13

// Encraption key
extern char arc_key;

// Initializes the internal archive state with the data from [fn]'s header.
// [fn] is closed at the end of the function, and assumed to remain unchanged
// for all arc_file_load() calls referring to any of its files, which will
// reopen this file.
void pascal arc_load(const char fn[PF_FN_LEN]);

// Frees all internal buffers allocated during arc_load().
void arc_free();

// Loads and decompresses or decrypts the given file inside the archive into
// the internal file buffer, resetting the cursor to the start of the file.
void pascal arc_file_load(const char fn[PF_FN_LEN]);

// Reads [size] bytes from the current position of the internal file buffer
// into [buf], advancing the cursor in the process.
void pascal arc_file_get(char *buf, size_t size);

#define arc_file_get_near(var) \
	arc_file_get(reinterpret_cast<char near *>(&var), sizeof(var))

#define arc_file_get_far(var) \
	arc_file_get(reinterpret_cast<char far *>(&var), sizeof(var))

// Sets the file cursor to the given [pos], relative to the start of the file.
void pascal arc_file_seek(char pos);

// Frees the internal file buffer.
void arc_file_free(void);
