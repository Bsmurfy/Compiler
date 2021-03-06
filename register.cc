#include "register.h"

Register::Register(int num)
{
	this->num = num;
	inuse = false;
}

Register::~Register()
{
}

int Register::get_num() const
{
	return num;
}

void Register::set_inuse()
{
	inuse = true;
}

void Register::clear_inuse()
{
	inuse = false;
}

bool Register::is_inuse() const
{
	return inuse;
}


