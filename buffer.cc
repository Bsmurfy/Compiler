#include "buffer.h"

Buffer::Buffer (char *filename)
{
	// Initialize the buffer
	this->buffer = new char[MAX_BUFFER_SIZE];
	this->current_buffer_size = 0;
	this->current_buffer_pos = 0;

    // Open the file and fill the buffer.
	source_file.open (filename);
	if (source_file.fail()) 
	{
		// Failed to open source file.
		cerr << "Can't open source file " << *filename << endl;
		buffer_fatal_error();
	}

  
}

Buffer::~Buffer()
{
	source_file.close();
}

char Buffer::current_char()
{
	return get_this_char();
}

char Buffer::pop_char()
{
	return pop_this_char();
}

void Buffer::unread_char(char c)
{
	this->current_buffer_size++;
	buffer[--this->current_buffer_pos] = c;
}

char Buffer::get_this_char()
{
  if (current_buffer_size < 1)
  {
     this->load_next_line();
  }

	return buffer[current_buffer_pos];
}

char Buffer::pop_this_char()
{
   // Load more chars if there are no characters in the buffer.
   if (current_buffer_size < 1)
   {
      this->load_next_line();
   }

	char cRetVal = this->get_this_char();
	this->current_buffer_size--;
	this->current_buffer_pos++;

	return cRetVal;
}


void Buffer::buffer_fatal_error() const
{
  cerr << "Exiting on BUFFER FATAL ERROR" << endl;
  exit (-1);
}


void Buffer::load_next_line()
{
	int read_size = 64;
	char* buffer_buffer = new char[read_size];

	// Read 64 characters from the file stream at a time
	source_file.read(buffer_buffer, read_size);

	// Load the buffer_buffer into the buffer
	for (int i = this->current_buffer_size; i < read_size; ++i)
	{
		this->buffer[i] = buffer_buffer[i];
	}

	// Reset the buffer overhead
	this->current_buffer_size += read_size;
	this->current_buffer_pos = 0;
  delete [] buffer_buffer;
}
