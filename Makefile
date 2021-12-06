all: lex yacc compiler 

OBJS =  generator.o \
		main.o	 \
		ast.o   \


LLVMCONFIG = llvm-config
CPPFLAGS = `$(LLVMCONFIG) --cxxflags --ldflags -std=c++11`
LIBS = `$(LLVMCONFIG) --system-libs --libs core`

clean:
	$(RM) -rf $(OBJS) compiler
	$(RM) -rf parser.out y.tab.cpp y.tab.hpp lex.yy.cpp y.output 
	$(RM) -rf output.ll output.bc output.o output.out output.json


ast.o: ast.cpp ast.h
#	clang++ $(CPPFLAGS) -o ast.cpp

genrator.o: generator.cpp generator.h
#	clang++ $(CPPFLAGS) -o generator.cpp

lex:
	flex -o lex.yy.cpp compiler.l

yacc:
	yacc -dy -o y.tab.cpp -v compiler.y

#parser: lex yacc
#	clang++ $(CPPFLAGS) $(LIBS) -o parser.out y.tab.cpp lex.yy.cpp ast.cpp generator.cpp parser.cpp

compiler: lex yacc $(OBJS)
	clang++ $(CPPFLAGS) -o $@ $(OBJS) $(LIBS) y.tab.cpp lex.yy.cpp

output:
	llvm-as output.ll
	llc -filetype=obj output.bc
	gcc output.o -o output.out -no-pie