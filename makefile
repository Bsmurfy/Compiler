token.o:	token.h token.cc
	g++ -c -g -Wall --pedantic token.cc

keywordtoken.o:	keywordtoken.h keywordtoken.cc token.h
	g++ -c -g -Wall --pedantic keywordtoken.cc

punctoken.o:	punctoken.h punctoken.cc token.h
	g++ -c -g -Wall --pedantic punctoken.cc

reloptoken.o:	reloptoken.h reloptoken.cc token.h
	g++ -c -g -Wall --pedantic reloptoken.cc

addoptoken.o:	addoptoken.h addoptoken.cc token.h
	g++ -c -g -Wall --pedantic addoptoken.cc

muloptoken.o:	muloptoken.h muloptoken.cc token.h
	g++ -c -g -Wall --pedantic muloptoken.cc

idtoken.o:	idtoken.h idtoken.cc token.h
	g++ -c -g -Wall --pedantic idtoken.cc

numtoken.o:	numtoken.h numtoken.cc token.h
	g++ -c -g -Wall --pedantic numtoken.cc

eoftoken.o:	eoftoken.h eoftoken.cc token.h
	g++ -c -g -Wall --pedantic eoftoken.cc

buffer.o:	buffer.h buffer.cc
	g++ -c -g -Wall --pedantic buffer.cc

scanner.o:	scanner.h scanner.cc buffer.h token.h keywordtoken.h \
		punctoken.h reloptoken.h addoptoken.h muloptoken.h \
		idtoken.h numtoken.h eoftoken.h
	g++ -c -g -Wall --pedantic scanner.cc

test_scanner.o:	test_scanner.cc scanner.h token.h keywordtoken.h \
		punctoken.h reloptoken.h addoptoken.h muloptoken.h \
		idtoken.h numtoken.h eoftoken.h
	g++ -c -g -Wall --pedantic test_scanner.cc

test_scanner:	test_scanner.o scanner.o buffer.o token.o keywordtoken.o \
		punctoken.o reloptoken.o addoptoken.o \
		muloptoken.o idtoken.o numtoken.o eoftoken.o 
	g++ -o test_scanner -g -Wall --pedantic scanner.o buffer.o eoftoken.o numtoken.o \
		idtoken.o muloptoken.o \
		addoptoken.o reloptoken.o punctoken.o keywordtoken.o \
		token.o test_scanner.o

register.o:	register.h register.cc
	g++ -c -g -Wall --pedantic register.cc

register_allocator.o: register_allocator.h register_allocator.cc register.h
	g++ -c -g -Wall --pedantic register_allocator.cc

emitter.o: emitter.h emitter.cc register.h
	g++ -c -g -Wall --pedantic emitter.cc

operand.o: operand.h operand.cc register.h
	g++ -c -g -Wall --pedantic operand.cc

parser.o:	parser.h parser.cc token.h keywordtoken.h \
		punctoken.h reloptoken.h addoptoken.h muloptoken.h \
		idtoken.h numtoken.h eoftoken.h scanner.h symbol_table.h
	g++ -c -g -Wall --pedantic parser.cc

truc.o:	truc.cc token.h keywordtoken.h \
		punctoken.h reloptoken.h addoptoken.h muloptoken.h \
		idtoken.h numtoken.h eoftoken.h scanner.h parser.h \
		register.h register_allocator.h emitter.h operand.h
	g++ -c -g -Wall --pedantic truc.cc

truc:	truc.o parser.o scanner.o buffer.o token.o keywordtoken.o \
		punctoken.o reloptoken.o addoptoken.o \
		muloptoken.o idtoken.o numtoken.o eoftoken.o \
		symbol_table.o register.o register_allocator.o emitter.o operand.o
	g++ -o truc -g -Wall --pedantic parser.o scanner.o buffer.o \
		symbol_table.o eoftoken.o numtoken.o \
		idtoken.o muloptoken.o \
		addoptoken.o reloptoken.o punctoken.o keywordtoken.o \
		token.o truc.o register.o register_allocator.o emitter.o operand.o

symbol_table.o: symbol_table.cc symbol_table.h
	g++ -c -g -Wall --pedantic symbol_table.cc

# A dependancy-less rule.  Always executes target when invoked.
clean:	
	rm *.o

# A target-less rule.  Used to test and rebuild anything listed
# in the dependency list.  
all:	token.o keywordtoken.o punctoken.o reloptoken.o addoptoken.o \
	muloptoken.o idtoken.o numtoken.o eoftoken.o \
  register.o register_allocator.o emitter.o operand.o
	buffer.o scanner.o test_scanner.o test_scanner \
  parser.o truc.o truc
