#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { SYMBOL, REFERENCE, LAMBDA, APPLICATION } objtype;
typedef struct obj {
	objtype type;
	unsigned int refcount;
	union {
		char *symbol;
		unsigned int reference;
		struct obj *expression;
		struct {
			struct obj *function;
			struct obj *argument;
		};
	};
} obj;

void
raiseerror(char *message) {
	fprintf(stderr, "Error: %s\n", message);
	exit(1);
}

void*
xmalloc(size_t size) {
	void *p;
	p = malloc(size);
	if(p == NULL)
		raiseerror("malloc returned NULL");
	return p;
}

void*
xrealloc(void *p, size_t size) {
	void *newp;
	newp = realloc(p, size);
	if(newp == NULL)
		raiseerror("realloc returned NULL");
	return newp;
}

obj*
allocobj(void) {
	return xmalloc(sizeof(obj));
}

obj*
apply(obj *function, obj *argument) {
	obj *application;
	
	application = allocobj();
	application->type = APPLICATION;
	application->refcount = 1;
	application->function = function;
	application->argument = argument;
	
	return application;
}

obj*
lambda(obj *expression) {
	obj *function;
	
	function = allocobj();
	function->type = LAMBDA;
	function->refcount = 1;
	function->expression = expression;
	
	return function;
}

obj*
reference(unsigned int depth) {
	obj *ref;
	
	ref = allocobj();
	ref->type = REFERENCE;
	ref->refcount = 1;
	ref->reference = depth;
	
	return ref;
}

obj*
symbol(char *symbol) {
	obj *sym;
	
	sym = allocobj();
	sym->type = SYMBOL;
	sym->refcount = 1;
	sym->symbol = symbol;
	
	return sym;
}

void
decreference(obj *object) {
	object->refcount--;
	if(object->refcount == 0) {
		switch(object->type) {
			case APPLICATION:
				decreference(object->function);
				decreference(object->argument);
				break;
			case LAMBDA:
				decreference(object->expression);
				break;
			default:
				break;
		}
		
		free(object);
	}
}

obj*
rewrite(obj *expression, obj *argument, unsigned int level) {
	obj *retvalue;
	
	switch(expression->type) {
		case APPLICATION:
			retvalue = apply(rewrite(expression->function, argument, level), rewrite(expression->argument, argument, level));
			break;
		case LAMBDA:
			retvalue = lambda(rewrite(expression->expression, argument, level + 1));
			break;
		case REFERENCE:
			if(expression->reference == level) {
				argument->refcount++;
				retvalue = argument;
			} else {
				expression->refcount++;
				retvalue = expression;
			}
			break;
		case SYMBOL:
			expression->refcount++;
			retvalue = expression;
			break;
	}
	
	return retvalue;
}

obj*
doapplication(obj *function, obj *argument) {
	if(function->type == LAMBDA) {
               return rewrite(function->expression, argument, 0);
	} else if(function->type == APPLICATION) {
		obj *inner = doapplication(function->function, function->argument);
		if(inner == NULL) /* Nothing was changed, thus we too return NULL */
			return NULL;
		argument->refcount++;
		return apply(inner, argument);
	} else {
		return NULL;
	}
}

void
printobj(obj *object) {
	switch(object->type) {
		case APPLICATION:
			putchar('`');
			printobj(object->function);
			printobj(object->argument);
			break;
		case LAMBDA:
			putchar('\\');
			printobj(object->expression);
			break;
		case REFERENCE:
			printf("%u ", object->reference);
			break;
		case SYMBOL:
			printf("%s ", object->symbol);
			break;
	}
}

void
print(obj *object) {
	printobj(object);
	putchar('\n');
}

void
mainloop(obj *program) {
	obj *head = program, *oldhead;
	
	print(program);
	
	while(head->type == APPLICATION) {
		oldhead = head;
		head = doapplication(head->function, head->argument);
		decreference(oldhead);
		if(head == NULL)
			break;
		print(head);
	}
}

bool
isnum(const char *token) {
	for(size_t i = 0; token[i] != 0; i++) {
		if(!isdigit(token[i]))
			return false;
	}
	
	return true;
}

obj*
readprog(void) {
	char *token = NULL;
	int c;
	size_t tokensize = 0;
	
	c = getchar();
	while(c >=0 && isspace(c))
		c = getchar();
	
	if(c == '`') {
		obj *lambda, *argument;
		lambda = readprog();
		argument = readprog();
		return apply(lambda, argument);
	} else if(c == '\\') {
		return lambda(readprog());
	}
	
	for(;;) {
		if(c<0)
			raiseerror("unexpected EOF");
		
		if(isspace(c))
			break;
		if(c == '`' || c == '\\') {
			ungetc(c, stdin);
			break;
		}
		
		token = xrealloc(token, ++tokensize);
		token[tokensize - 1] = c;
		
		c = getchar();
	}
	token = xrealloc(token, ++tokensize);
	token[tokensize - 1] = 0;
	
	if(isnum(token)) {
		return reference(strtoul(token, NULL, 10));
	} else {
		return symbol(token);
	}
}

int
main(void) {
	obj *program;
	
	program = readprog();
	mainloop(program);
	
	return 0;
}
