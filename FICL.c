/* Copyright (C) 2013 paul@marrington.net, see GPL for license */
/* Language Selection */
#include "FICL.h"

package_local int RUNNING_WORD_STACK_DEPTH = 32;
package_local int COMPILING_STACK_DEPTH = 16;
package_local int COMPILING_WORD_DEPTH = 64;
package_local int OPERATING_STACK_DEPTH = 128;
package_local int SECOND_STACK_DEPTH = 16;
package_local int LOOP_STACK_DEPTH = 32;


/* CompiledWord Implementation */
CompiledWord newCompiledWord(String name, runnable(actor)) {
    CompiledWord compiledWord = allocate(CompiledWord);
    compiledWord(name) = name;
    compiledWord(actor) = actor;
    compiledWord(immediate) = compiledWord(variable) = false;
    return compiledWord;
}
package_local boolean isSpace(char c) {
    return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
}
package_local char charAt(String text, int at) {
    return text[at];
}
package_local String getWord(FICL ficl) {
    /* start by dropping any leading space characters */
    do {
        if (ficl(sourcePointer) == ficl(sourceLength)) return null;
    } while (isSpace(charAt(ficl(source), ficl(sourcePointer)++)));
    int first = ficl(sourcePointer) - 1;
    // and go to where we have a space again
    do {
        if (ficl(sourcePointer) == ficl(sourceLength))
            return (substr(ficl(source), first));
    } while (!isSpace(charAt(ficl(source), ficl(sourcePointer)++)));
    
    String word = substr(ficl(source), first, ficl(sourcePointer) - 1);
    if (ficl(debuggingCompile)) print(strcat(word, ' '));
    return word;
}
package_local compileWord(FICL ficl, CompiledWord word) {
    pushOnStack(ficl(compiling), word);
}
package_local boolean compileLiteral(String name) {
    int num = 0, sign = -1, idx = 0, len = strlen(name);
    if (charAt(name, 0) == '-') {
        sign = idx = 1;
    }
    int decimal = -10000;
    while (idx < len) {
        int chr = charAt(name, idx++);
        if (chr == '.') {
            decimal = 1;
        } else {
            int digit = '0' - chr;
            if (digit > 0 || digit < -9) return false; // NaN
            num = (num * 10) + digit;
            decimal *= 10;
        }
    }
    if (decimal < 0) {
        compilePushWord(name, toInteger(sign * num));
    } else {
        double real = (sign * num) / (double) decimal;
        compilePushWord(name, toDouble(real));
    }
    return true;

}
package_local runCode(CompiledWord word) {
    word.actor();
}
package_local FICL allocateFICL() {
    return ref(new sFICL());
}
/* FICL API implementation */
public CompiledWord compileFICL(FICL ficl, String source) {
    ficl.isCompileMode = true;
    ficl.writer = newStringBuffer();
    ficl.abort = false;
    ficl.source = source;
    ficl.sourceLength = strlen(source);
    ficl.sourcePointer = 0;
    ficl.compiling.depth = 0;
    ficl.errors.newStringBuffer();
    String name;
    while (!abort && ((name = getWord(ficl)) != null)) {
        if (dictionary.containsKey(name)) {
            CompiledWord word = (CompiledWord) getFromDictionary(dictionary, name);
            // we call the word in compile mode
            if (word.immediate)
                runCode(word);
            else
                compileWord(word);
        } else if (!compileLiteral(name)) {
            abort(strcat("Unknown word: ", name));
            break;
        }
    }
    compilingWord = "[[compiled]]";
    compiled = new WordOfWords();
    ficl.isCompileMode = false;
    return !ficl.abort;
}
public String runFICL(FICL ficl, CompiledWord word) {
    pushOnStack(ficl.runningWords, word.name);
    runCode(word);
    popFromStack(ficl.runningWords)
}
public reset(FICL ficl) {
    ficl.stack.depth = ficl.secondStack.depth = ficl.loopStack.depth =
    ficl.compilingStack.depth = ficl.runningWords.depth = 0;
}
public CompiledWord extendFICL(FICL ficl, String name, Runnable action) {
    CompiledWord word = newCompiledWord(name, action);
    addToDictionary(ficl.dictionary, name, word);
    return word;
}
public void immediateFICL(FICL ficl, String name, Runnable action) {
    extend(ficl, name, action).immediate = true;
}
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
package_local _colon(ficl) {
    pushOnStack(ficl.secondStack, ficl.compilingWord);
    pushOnStack(ficl.secondStack, toInteger(ficl.sourcePointer));
    ficl.compilingWord = getWord();
    pushOnStack(ficl.compilingStack, ficl.compiling);
    ficl.compiling = newStack(COMPILING_WORD_DEPTH);
}
package_local _semicolon() {
    int start = toInt(popFromStack(ficl.secondStack))
    CompiledWord compiledWord =
      newCompiledWord(ficl.compilingWord, newWordOfWords());
    compiledWord.source =
      substr(ficl.source, start, ficl.sourcePointer);
    
    //noinspection unchecked
    compiling = (Stack) popFromStack(ficl.compilingStack);
    ficl.lastDefinition = compilingWord;
    compilingWord = (String) popFromStack(ficl.secondStack);
    addToDictionary(ficl.lastDefinition, compiledWord);
}
package_local _open_bracked() {
    ficl.sourcePointer = strind(ficl.source, ')', ficl.sourcePointer) + 1;
    if (ficl.sourcePointer == 0) ficl.sourcePointer = ficl.sourceLength;
}
package_local _slosh() {
    ficl.sourcePointer = ficl.sourceLength;
}
package_local _double_quote() {
    compilePushWord("\"", getQuotedString());
}
package_local _dot() {
    print(popFromStack(stack));
}
package_local _and() {
    pushOnStack(ficl.stack,
      toInteger(popIntFromStack(ficl.stack) & popIntFromStack(ficl.stack)));
}
package_local _plus() {
    pushOnStack(ficl.stack,
      toInteger(popIntFromStack(ficl.stack) + popIntFromStack(ficl.stack)));
}
package_local _begin() {
    int[] beginLeaveEnd = {ficl.compiling.depth, 0};
    pushOnStack(loopStack, beginLeaveEnd);
}
package_local _leave() {
    int[] beginLeaveEnd = (int[]) peekIntoStack(ficl.loopStack);
    compileWord("leave() {
            jumpTo = beginLeaveEnd[1];
        }
    });
}
package_local _qleave() {
    int[] beginLeaveEnd = (int[]) peekIntoStack(ficl.loopStack);
    compileWord("?leave() {
            int testResult = ((Integer) stack.pop()).intValue();
            if (testResult == 0) {
                jumpTo = beginLeaveEnd[1];
            }
    });
}
package_local _again() {
    final int[] beginLeaveEnd = (int[]) popFromStack(ficl.loopStack);
    beginLeaveEnd[1] = ficl.compiling.depth + 1;
    compileWord("again() {
            ficl.jumpTo = beginLeaveEnd[0];
        }
    });
}
package_local _constant() {
    runCode((CompiledWord) getFromDictionary(ficl.dictionary, ficl.lastDefinition));
    final Object constant = ficl.stack.pop();
    CompiledWord word =
      compiledPushWord(ficl.lastDefinition + " - constant", constant));
    addToDictionary(ficl.dictionary, ficl.lastDefinition, word);
}
package_local _dec() {
    pushOnStack(ficl.stack, toInteger(popIntFromStack(stack)) - 1);
}
package_local _debug_compile() {
    ficl.debuggingCompile = true;
}
package_local _divide() {
    pushOnStack(ficl.stack,
      toInteger(popIntFromStack(stack) / popIntFromStack(stack)));
}
package_local _drop() {
    ficl.stack.pop();
}
package_local _dup() {
    ficl.stack.push(peekIntoStack(stack));
}
package_local _equals() {
    int a = popIntFromStack(ficl.stack)
    int b = popIntFromStack(ficl.stack)
    int equals = (a == b)
    pushOnStack(ficl.stack, toInteger(equals));
}
package_local _if() {
    final int[] ifElseNext = {ficl.compiling.depth, 0, 0};
    ficl.secondStack.push(ifElseNext);
    compileWord("if() {
            int testResult = popIntFromStack(ficl.stack);
            if (testResult == 0) {
                ficl.jumpTo = ifElseNext[1];
            }
        }
    });
}
package_local _else() {
    final int[] ifElseNext = (int[]) peekIntoStack(ficl.secondStack);
    ifElseNext[1] = ficl.compiling.depth + 1; // over else
    compileWord("else() {
            ficl.jumpTo = ifElseNext[2];
        }
    });
}
package_local _then() {
    int[] ifElseNext = (int[]) ficl.secondStack.pop();
    // make sure we run the then statement to clean up secondStack
    ifElseNext[2] = ficl.compiling.depth;
    if (ifElseNext[1] == 0) ifElseNext[1] = ifElseNext[2];
}
package_local _immediate() {
    ((CompiledWord) getFromDictionary(
      ficl.dictionary, ficl.lastDefinition)).immediate = true;
}
package_local _inc() {
    pushOnStack(ficl.stack, toInteger(popIntFromStack(stack) + 1));
}
package_local _minus() {
    pushOnStack(ficl.stack, toInteger(popIntFromStack(ficl.stack) -
                popIntFromStack(ficl.stack)));
}
package_local _multiply() {
    pushOnStack(ficl.stack, toInteger(
      popIntFromStack(ficl.stack) * popIntFromStack(ficl.stack)));
}
package_local _not() {
    pushOnStack(ficl.stack, popIntFromStack(ficl.stack) == 0));
}
package_local _or() {
    pushOnStack(ficl.stack, toInteger(
      (popIntFromStack(ficl.stack) | popIntFromStack(ficl.stack)));
}
package_local _push() {
    compilePushWord("push", getFromDictionary(ficl.dictionary, getWord()));
}
package_local _random() {
    int i = random.nextInt(popIntFromStack(ficl.stack));
    ficl.stack.push(toInteger(i));
}
package_local _return() {
    ficl.jumpTo = 100000;
}
package_local _variable() {
    final CompiledWord word = (CompiledWord)
      getFromDictionary(dictionary, lastDefinition);
    if (!word.variable) {
        addToDictionary(dictionary, lastDefinition,
          newVariableWord(ficl.lastDefinition + " - variable", word));
        addToDictionary(dictionary, lastDefinition, word);   // matches second
    }
}
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
package_local void init() {
    immediateFICL(ficl, ":", _colon);
    immediateFICL(";", _semicolon);
    immediateFICL("(", _open_bracked);
    immediateFICL("\\", _slosh);
    immediateFICL("\"", _double_quote);
    extendFICL(".", _dot);
    extendFICL("and", _and);
    extendFICL("+", _plus);
    immediateFICL("begin", _begin);
    immediateFICL("leave", _leave);
    immediateFICL("?leave", _qleave);
    immediateFICL("again", _again);
    immediateFICL("constant", _constant);
    extendFICL("dec", _dec);
    immediateFICL("debug-compile", _debug_compile);
    extendFICL("/", _divide);
    extendFICL("drop", _drop);
    extendFICL("dup", _dup);
    extendFICL("=", _equals);
    immediateFICL("if", _if);
    immediateFICL("else", _else);
    immediateFICL("then", _then);
    immediateFICL("immediate", _immediate);
    extendFICL("inc", _inc);
    extendFICL("-", _minus);
    extendFICL("*", _star);
    extendFICL("not", _not);
    extendFICL("or", _or);
    immediateFICL("push", _push);
    extendFICL("random", _random);
    extendFICL("return", _return);
    immediateFICL("variable", _variable);
}
////////////////////////////////////////////////////////
public FICL newFICL() {
    FICL ficl = allocateFICL();
    ficl.dictionary = newDictionary();
    ficl.compiling = newStack(COMPILING_WORD_DEPTH);
    ficl.secondStack = newStack(SECOND_STACK_DEPTH);
    ficl.stack = newStack(OPERATING_STACK_DEPTH);
    ficl.compilingStack = newStack(COMPILING_STACK_DEPTH);
    ficl.loopStack = newStack(LOOP_STACK_DEPTH);
    init(ficl);
    return ficl;
}
