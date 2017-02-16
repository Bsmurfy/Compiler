#ifndef REGISTER_H
#define REGISTER_H

// To get STL stack
#include <stack>
#include <iostream>

#include <stdlib.h>

using namespace std;

class Register
{
public:
	Register(int num);
	~Register();

	int get_num() const;

	// Mark the register as inuse and unavailable for allocation
	void set_inuse();
	// Mark the register as free and available for allocation
	void clear_inuse();

	bool is_inuse() const;

private:
	int num;
	bool inuse;

};
#endif
	
	

