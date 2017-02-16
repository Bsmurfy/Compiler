#ifndef RelopToken_H
#define RelopToken_H

#include "reloptoken.h"

using namespace std;

RelopToken::RelopToken()
{
	this->set_token_type(TOKEN_RELOP);
}

RelopToken::~RelopToken()
{

}

RelopToken::RelopToken(relop_attr_type   attr) : RelopToken()
{
	this->set_attribute(attr);
}

void RelopToken::set_attribute(relop_attr_type   attr)
{
	this->attribute = attr;
}

relop_attr_type  RelopToken::get_attribute()
{
	return this->attribute;
}

string* RelopToken::to_string() const
{
	string szRetVal = "";
	if (this->attribute == RELOP_EQ)
	{
		szRetVal = "=";
	}
	else if (this->attribute == RELOP_GE)
	{
		szRetVal = ">=";
	}
	else if (this->attribute == RELOP_LE)
	{
		szRetVal = "<=";
	}
	else if (this->attribute == RELOP_LT)
	{
		szRetVal = "<";
	}
	else if (this->attribute == RELOP_GT)
	{
		szRetVal = ">";
	}
	else if (this->attribute == RELOP_NE)
	{
		szRetVal = "<>";
	}
	return new string(szRetVal);
}

#endif
