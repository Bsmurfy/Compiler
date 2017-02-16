#ifndef PUNCTOKEN_H
#define PUNCTOKEN_H

#include "numtoken.h"

using namespace std;

NumToken::NumToken()
{
	this->set_token_type(TOKEN_NUM);
}

NumToken::~NumToken()
{

}

NumToken::NumToken(const string& attr) : NumToken()
{
	
	this->set_attribute(attr);
}

void NumToken::set_attribute(const string& attr)
{
	this->attribute = new string(attr);
}

string* NumToken::get_attribute()
{
	return this->attribute;
}

string* NumToken::to_string() const
{
	return this->attribute;
}

#endif