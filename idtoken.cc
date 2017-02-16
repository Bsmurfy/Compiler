#ifndef IdToken_H
#define IdToken_H

#include "idtoken.h"

using namespace std;

IdToken::IdToken()
{
	this->set_token_type(TOKEN_ID);
}

IdToken::~IdToken()
{

}

IdToken::IdToken(const string&  attr) : IdToken()
{
	this->set_attribute(attr);
}

void IdToken::set_attribute(const string& attr)
{
	this->attribute = new string(attr);
}

string*  IdToken::get_attribute()
{
	return this->attribute;
}

string* IdToken::to_string() const
{
	return this->attribute;
}

#endif