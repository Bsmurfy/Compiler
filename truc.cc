#include <iostream>
#include <stdlib.h>

#include "scanner.h"
#include "parser.h"

using namespace std;

int main (int argc, char **argv)
{

  char *filename;

  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " <input file name>" << endl;
    exit (-1);
  }

  filename = argv[1];

  // Declare a Scanner object and a Parser object.
  Scanner s(filename);
  Parser p(&s);

  if (p.parse_program()){
    cout << "Parse successful." << endl;
  } else {
    cout << "Parse failed."    << endl;
  }

}
