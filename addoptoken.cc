#ifndef AddopToken_H
#define AddopToken_H

#include "addoptoken.h"

using namespace std;

AddopToken::AddopToken()
{
	this->set_token_type(TOKEN_ADDOP);
}

AddopToken::~AddopToken()
{

}

AddopToken::AddopToken(addop_attr_type   attr) : AddopToken()
{
	this->set_attribute(attr);
}

void AddopToken::set_attribute(addop_attr_type   attr)
{
	this->attribute = attr;
}

addop_attr_type  AddopToken::get_attribute()
{
	return this->attribute;
}

string *AddopToken::to_string() const
{
  string szKWAddop = "";
  if (this->attribute == ADDOP_ADD)
  {
     szKWAddop = "ADDOP_ADD";
  }
  else if (this->attribute == ADDOP_OR)
  {
     szKWAddop = "ADDOP_OR";
  }
  else if (this->attribute == ADDOP_SUB)
  {
     szKWAddop = "ADDOP_SUB";
  } else {
     szKWAddop = "ADDOP_NO_ATTR";
  }
	return new string(szKWAddop);
}

#endif
